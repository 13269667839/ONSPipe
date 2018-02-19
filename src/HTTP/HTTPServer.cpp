#include "HTTPServer.hpp"
#include "../Utility/HTTPParser.hpp"

#ifdef Kqueue
    #define MAX_EVENT_COUNT 64

    #include <sys/event.h>
    #include <iostream>
#elif defined(Select)
    #include <sys/select.h>
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
    
    mainLoop(callback);
}

void HTTPServer::mainLoop(const RunAndLoopCallback &callback)
{
#ifdef Kqueue
    kqueueLoop(callback);
#elif defined(Select)
    selectLoop(callback);
#elif defined(Epoll)
    epollLoop(callback);
#endif
}

#pragma mark -- Kqueue
#ifdef Kqueue
void HTTPServer::kqueueLoop(const RunAndLoopCallback &callback)
{
    int listenfd = sock->socketfd;
    if (listenfd == -1)
    {
        return;
    }
    
    int kq = kqueue();
    if (kq == -1)
    {
        throwError("kqueue() error : " + std::string(gai_strerror(errno)));
        return;
    }
    
    struct kevent listen_event = {static_cast<unsigned long>(listenfd),EVFILT_READ,EV_ADD,0,0,nullptr};
    if (kevent(kq, &listen_event, 1, nullptr, 0, nullptr) == -1)
    {
        throwError("kevent() error : " + std::string(gai_strerror(errno)));
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
                int clientfd = sock->accept();
                if (clientfd > 0)
                {
                    struct kevent changelist[2];
                    EV_SET(&changelist[0], clientfd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
                    EV_SET(&changelist[1], clientfd, EVFILT_WRITE, EV_ADD, 0, 0, nullptr);
                    kevent(kq, changelist, 1, nullptr, 0, nullptr);
                }
                continue;
            }
            
            if (event.flags & EV_ERROR)//error event
            {
                sock->close(sockfd);
                struct kevent event = {static_cast<unsigned long>(sockfd),EVFILT_READ,EV_DELETE,0,0,nullptr};
                kevent(kq, &event, 1, nullptr, 0, nullptr);
                std::cout<<"socket broken,error : "<<event.data<<std::endl;
                continue;
            }
            else
            {
                if (event.filter == EVFILT_READ)//read event
                {
                    request.initParameter();
                    parser.initParams();
                    
                    while (true)
                    {
                        void *recvBuf = nullptr;
                        long bytes = -2;
                        
                        std::tie(recvBuf,bytes) = sock->receive(sockfd);
                        
                        if (!recvBuf)
                        {
                            if (bytes == 0 || bytes == -1)
                            {
                                sock->close(sockfd);
                                sockfd = -1;
                                struct kevent event = {static_cast<uintptr_t>(sockfd),EVFILT_READ,EV_DELETE,0,0,NULL};
                                kevent(kq, &event, 1, NULL, 0, NULL);
                            }
                            break;
                        }
                        
                        auto strBuf = static_cast<Util::byte *>(recvBuf);
                        
                        if (!strBuf)
                        {
                            break;
                        }
                        
                        if (!parser.cache)
                        {
                            parser.cache = new std::deque<Util::byte>();
                        }
                        
                        for (int i = 0;i < bytes;++i)
                        {
                            parser.cache->push_back(strBuf[i]);
                        }
                        
                        delete strBuf;
                        strBuf = nullptr;
                        
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
                    }
                }
            }
        }
    }
}
#endif

#pragma mark -- Select
#ifdef Select
void HTTPServer::selectLoop(const RunAndLoopCallback &callback)
{
    auto read_fds = fd_set();
    FD_ZERO(&read_fds);
    
    auto master = fd_set();
    FD_ZERO(&master);
    
    const auto listener = sock->socketfd;
    FD_SET(listener,&master);
    
    auto fd_max = listener;
    
    auto request = HTTPRequest();
    auto response = HTTPResponse();
    auto parser = HTTPReqMsgParser();
    
    while (true)
    {
        read_fds = master;
        
        if (select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            Util::throwError(std::string(gai_strerror(errno)));
            return;
        }

        for (int i = 0;i <= fd_max;++i)
        {
            if (!FD_ISSET(i,&read_fds))
            {
                continue;
            }

            if (i == listener)
            {
                int fd = sock->accept();

                if (fd != -1)
                {
                    FD_SET(fd, &master);
                    fd_max = std::max(fd, fd_max);
                }
            }
            else
            {
                request.initParameter();
                parser.initParams();
                while (true)
                {
                    void *recvBuf = nullptr;
                    long bytes = -2;

                    std::tie(recvBuf,bytes) = sock->receive(i);

                    if (!recvBuf)
                    {
                        if (bytes == 0 || bytes == -1)
                        {
                            FD_CLR(i,&master);
                        }
                        break;
                    }

                    auto strBuf = static_cast<Util::byte *>(recvBuf);

                    if (!strBuf)
                    {
                        break;
                    }

                    if (!parser.cache)
                    {
                        parser.cache = new std::deque<Util::byte>();
                    }
                    
                    for (int i = 0;i < bytes;++i)
                    {
                        parser.cache->push_back(strBuf[i]);
                    }
                    
                    delete strBuf;
                    strBuf = nullptr;
                    
                    if (parser.is_parse_msg())
                    {
                        parser.msg2req(request);
                        break;
                    }
                }

                response.initParameter();
                callback(request,response);
                sock->sendAll(response.toResponseMessage(),i);
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
                    void *recvBuf = nullptr;
                    long bytes = -2;
                        
                    std::tie(recvBuf,bytes) = sock->receive(sockfd);

                    if (!recvBuf)
                    {
                        if (bytes == 0 || bytes == -1)
                        {
                            sock->close(sockfd);
                            sockfd = -1;
                            events[i].data.fd = -1;
                        }
                        break;
                    }
                        
                    auto strBuf = static_cast<Util::byte *>(recvBuf);
                        
                    if (!strBuf)
                    {
                        break;
                    }
                        
                    if (!parser.cache)
                    {
                        parser.cache = new std::deque<Util::byte>();
                    }
                        
                    for (int i = 0;i < bytes;++i)
                    {
                        parser.cache->push_back(strBuf[i]);
                    }
                        
                    delete strBuf;
                    strBuf = nullptr;
                        
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
                    sock->sendAll(response.toResponseMessage(),sockfd);

                    ev.data.fd = sockfd;
                    ev.events = EPOLLOUT | EPOLLET;
                    epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
                }
            }
        }
    }
}
#endif
