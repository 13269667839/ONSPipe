#include "HTTPServer.hpp"

#ifdef Kqueue
    #define MAX_EVENT_COUNT 64

    #include <sys/event.h>
#elif defined(Epoll)
    #include <sys/epoll.h>
#endif

HTTPServer::HTTPServer(int port)
{
    this->port = port;
    sock = nullptr;
}

HTTPServer::~HTTPServer()
{
    if (sock)
    {
        sock->close();
        delete sock;
        sock = nullptr;
    }
}

void HTTPServer::disconnect(int sockfd)
{
    if (sock && sockfd > 0)
    {
        sock->close(sockfd);
    }
}

void HTTPServer::setSocket()
{
    if (sock)
    {
        return;
    }
    
    sock = new Socket("",port);
    
    int yes = 1;
    sock->setSocketOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
    if (!(sock->bind() && sock->listen()))
    {
        throwError("some error occur on bind or listen function");
    }
}

void HTTPServer::runAndLoop(RunAndLoopCallback callback)
{
    if (!callback)
    {
        return;
    }
    
    setSocket();
    
#ifdef Kqueue
    kqueueLoop(callback);
#elif defined(Epoll)
    epollLoop(callback);
#endif
}

#pragma mark -- Kqueue
#ifdef Kqueue
std::tuple<int,int> HTTPServer::setupKqueue()
{
    auto listenfd = sock->socketfd;
    if (listenfd == -1)
    {
        throwError("listen fd is invalid");
    }

    auto kq = kqueue();
    if (kq == -1)
    {
        throwError("kqueue() error : " + std::string(gai_strerror(errno)));
    }
    
    struct kevent listen_event = {static_cast<unsigned long>(listenfd),EVFILT_READ,EV_ADD,0,0,nullptr};
    if (kevent(kq, &listen_event, 1, nullptr, 0, nullptr) == -1)
    {
        throwError("kevent() error : " + std::string(gai_strerror(errno)));
    }

    return std::make_tuple(listenfd,kq);
}

void HTTPServer::kqueueAccept(long count,int kq)
{
    for (auto i = 0;i < count;++i)
    {
        auto clientfd = sock->accept();
        if (clientfd > 0)
        {
            struct kevent changelist[2];
            EV_SET(&changelist[0], clientfd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
            EV_SET(&changelist[1], clientfd, EVFILT_WRITE, EV_ADD, 0, 0, nullptr);
            kevent(kq, changelist, 1, nullptr, 0, nullptr);
        }
    }
}

void HTTPServer::kqueueError(int sockfd,int kq)
{
    struct kevent event = {static_cast<unsigned long>(sockfd),EVFILT_READ,EV_DELETE,0,0,nullptr};
    kevent(kq, &event, 1, nullptr, 0, nullptr);
}

void HTTPServer::kqueueSend(int sockfd,HTTPRequest &request,HTTPResponse &response,RunAndLoopCallback &callback)
{
    response.initParameter();
    callback(request,response);
    auto msg = response.toResponseMessage();
    sock->sendAll(const_cast<char *>(msg.c_str()),msg.size(),false,sockfd);
}

bool HTTPServer::kqueueParseRecvRequest(HTTPRequest &request, HTTPReqMsgParser &parser, long totalLength, int sockfd)
{
    auto empty = false;

    request.initParameter();
    parser.initParams();

    long recvBytes = 0;
    auto recvBuf = std::vector<Util::byte>();
    while (recvBytes <= totalLength)
    {
        auto size = totalLength - recvBytes;
        if (size > 0) 
        {
            sock->recvBuffSize = size;
        }

        try 
        {
            auto buff = sock->receive(sockfd);
            recvBuf.insert(recvBuf.end(),buff.begin(),buff.end());
        }
        catch (std::logic_error error)
        {
            #ifdef DEBUG
            std::cerr<<error.what()<<endl;
            #endif

            empty = true;
            parser.msg2req(request);
            break;
        }

        empty = recvBuf.empty();
        if (!empty) 
        {
            recvBytes += recvBuf.size();
            parser.addToCache(recvBuf);
            recvBuf.clear();
        }

        auto parsed = parser.is_parse_msg();

        if (empty || parsed)
        {
            parser.msg2req(request);
            break;
        }
    }

    return empty;
}

void HTTPServer::kqueueLoop(RunAndLoopCallback &callback)
{
    int listenfd = -1;
    int kq = -1;
    std::tie(listenfd,kq) = setupKqueue();
    if (listenfd == -1 || kq == -1) 
    {
        throwError("setupKqueue() error");
        return;
    }
    
    auto request = HTTPRequest();
    auto response = HTTPResponse();
    auto parser = HTTPReqMsgParser();
    
    while (true)
    {
        struct kevent eventlist[MAX_EVENT_COUNT];
        int ret = kevent(kq, nullptr, 0, eventlist, MAX_EVENT_COUNT, nullptr);
        if (ret <= 0)
        {
            continue;
        }
            
        for (int i = 0;i < ret;i++)
        {
            auto event = eventlist[i];
            auto sockfd = static_cast<int>(event.ident);
            
            if (sockfd == listenfd)//new client
            {
                kqueueAccept(event.data,kq);
                continue;
            }
            
            if (event.flags & EV_ERROR)//error event
            {
                kqueueError(sockfd,kq);
                disconnect(sockfd);
                continue;
            }

            if (event.filter == EVFILT_READ && sockfd > 0) //read event
            {
                auto empty = kqueueParseRecvRequest(request, parser, event.data, sockfd);

                if (!empty)
                {
                    kqueueSend(sockfd, request, response, callback);
                }

                if (empty || request.header->find("Connection")->second == "Close")
                {
                    kqueueError(sockfd, kq);
                    disconnect(sockfd);
                }
            }
        }
    }
}
#endif

#pragma mark -- Epoll
#ifdef Epoll
void HTTPServer::epollLoop(const RunAndLoopCallback &callback)
{
    //用于回传要处理的事件
    epoll_event events[20];
    //生成用于处理accept的epoll专用的文件描述符
    auto epfd = epoll_create(256);

    const auto listenfd = sock->socketfd;

    //用于注册事件
    epoll_event ev;
    //设置与要处理的事件相关的文件描述符
    ev.data.fd = listenfd;
    //设置要处理的事件类型
    ev.events = EPOLLIN | EPOLLET;

    //注册epoll事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    auto request = HTTPRequest();
    auto response = HTTPResponse();
    auto parser = HTTPReqMsgParser();

    while (1)
    {
        //等待epoll事件的发生
        auto nfds = epoll_wait(epfd, events, 20, 500);

        //处理所发生的所有事件
        for (auto i = 0;i < nfds;++i)
        {
            if (events[i].data.fd == listenfd) 
            {
                int connfd = sock->accept();
                if (connfd > 0)
                {
                    //设置用于读操作的文件描述符
                    ev.data.fd = connfd;
                    //设置用于注测的读操作事件
                    ev.events = EPOLLIN | EPOLLET;
                    //注册ev
                    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                auto sockfd = events[i].data.fd;
                if (sockfd < 0)
                {
                    continue;
                }

                request.initParameter();
                parser.initParams();
                while (true)
                {
                    auto recvBuf = sock->receive(sockfd);

                    if (recvBuf.empty())
                    {
                        // if (bytes == -1)
                        // {
                        //     sock->close(sockfd);
                        //     sockfd = -1;
                        //     events[i].data.fd = -1;
                        // }
                        parser.msg2req(request);
                        break;
                    }
                        
                    parser.addToCache(recvBuf);
                    if (parser.is_parse_msg())
                    {
                        parser.msg2req(request);
                        break;
                    }
                }
                    
                if (sockfd != -1)
                {
                    response.initParameter();
                    callback(request,response);
                    auto msg = response.toResponseMessage();
                    sock->sendAll(const_cast<char *>(msg.c_str()),msg.size(),false,sockfd);

                    ev.data.fd = sockfd;
                    ev.events = EPOLLOUT | EPOLLET;
                    epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
                }
            }
        }
    }
}
#endif
