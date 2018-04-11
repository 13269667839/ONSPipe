#include "HTTPServer.hpp"

#ifdef Kqueue

constexpr int MaxEventCount = 10;

#include <sys/event.h>

#elif defined(Epoll)
#include <sys/epoll.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

HTTPServer::HTTPServer(int port)
{
    this->port = port;
    sock = nullptr;
    listenfd = -1;
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

    sock = new Socket("", port);

    int yes = 1;
    sock->setSocketOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (!(sock->bind() && sock->listen()))
    {
        throwError("some error occur on bind or listen function");
    }

    listenfd = sock->socketfd;
    if (listenfd == -1)
    {
        throwError("listen fd is -1");
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

#pragma mark-- Kqueue
#ifdef Kqueue
int HTTPServer::setupKqueue()
{
    auto kq = kqueue();
    if (kq == -1)
    {
        throwError("kqueue() error : " + std::string(gai_strerror(errno)));
    }

    struct kevent listenEvent;
    EV_SET(&listenEvent, listenfd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    if (kevent(kq, &listenEvent, 1, nullptr, 0, nullptr) == -1)
    {
        throwError("kevent() error : " + std::string(gai_strerror(errno)));
    }

    return kq;
}

void HTTPServer::kqueueAccept(long count, int kq)
{
    for (auto i = 0; i < count; ++i)
    {
        auto clientfd = sock->accept();
        if (clientfd > 0)
        {
            struct kevent changelist[2];
            EV_SET(&changelist[0], clientfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
            EV_SET(&changelist[1], clientfd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
            kevent(kq, changelist, 2, nullptr, 0, nullptr);
        }
    }
}

void HTTPServer::deleteEvent(int sockfd, int kq, int eventType)
{
    struct kevent event;
    EV_SET(&event, sockfd, eventType, EV_DELETE, 0, 0, nullptr);
    kevent(kq, &event, 1, nullptr, 0, nullptr);
}

//0 : normal 1 : empty 2 : error
int HTTPServer::kqueueParseRecvRequest(HTTPRequest &request, HTTPReqMsgParser &parser, long totalLength, int sockfd)
{
    int status = 0;

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
            recvBuf.insert(recvBuf.end(), buff.begin(), buff.end());
        }
        catch (std::logic_error error)
        {
#ifdef DEBUG
            std::cerr << error.what() << std::endl;
#endif

            status = 2;
            parser.msg2req(request);
            break;
        }

        if (recvBuf.empty())
        {
            status = 1;
            parser.msg2req(request);
            break;
        }

        recvBytes += recvBuf.size();
        parser.addToCache(recvBuf);
        recvBuf.clear();

        if (parser.is_parse_msg())
        {
            parser.msg2req(request);
            break;
        }
    }

    return status;
}

void HTTPServer::kqueueLoop(RunAndLoopCallback &callback)
{
    auto kq = setupKqueue();
    auto request = HTTPRequest();
    auto response = HTTPResponse();
    auto parser = HTTPReqMsgParser();
    struct kevent eventlist[MaxEventCount];
    
    while (true)
    {
        auto activeEventCount = kevent(kq, nullptr, 0, eventlist, MaxEventCount, nullptr);

        for (auto i = 0; i < activeEventCount; i++)
        {
            auto event = eventlist[i];
            auto sockfd = static_cast<int>(event.ident);

            if (event.flags & EV_ERROR || sockfd <= 0) //error event
            {
                deleteEvent(sockfd, kq, event.filter);

                if (i + 1 == activeEventCount)
                {
                    disconnect(sockfd);
                }

                continue;
            }

            if (sockfd == listenfd) //new client
            {
                kqueueAccept(event.data, kq);
                continue;
            }

            if (event.filter == EVFILT_READ) //read event
            {
                auto status = kqueueParseRecvRequest(request, parser, event.data, sockfd);
                switch (status)
                {
                case 0:
                {
                    response.initParameter();
                    callback(request, response);

                    auto message = response.toResponseMessage();
                    sock->sendAll(const_cast<char *>(message.c_str()), message.size(), false, sockfd);

                    if (request.header->find("Connection")->second == "Close")
                    {
                        deleteEvent(sockfd, kq, EVFILT_READ);
                        disconnect(sockfd);
                    }
                }
                break;
                case 1 ... 2:
                    deleteEvent(sockfd, kq, EVFILT_READ);
                    disconnect(sockfd);
                    break;
                default:
                    break;
                }
            }
            else if (event.filter == EVFILT_WRITE) //write event
            {
                deleteEvent(sockfd, kq, EVFILT_WRITE);
            }
        }
    }
}
#endif

#pragma mark-- Epoll
#ifdef Epoll
void HTTPServer::epollLoop(const RunAndLoopCallback &callback)
{
    //用于回传要处理的事件
    epoll_event events[20];
    //生成用于处理accept的epoll专用的文件描述符
    auto epfd = epoll_create(256);

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
        for (auto i = 0; i < nfds; ++i)
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
                    callback(request, response);
                    auto msg = response.toResponseMessage();
                    sock->sendAll(const_cast<char *>(msg.c_str()), msg.size(), false, sockfd);

                    ev.data.fd = sockfd;
                    ev.events = EPOLLOUT | EPOLLET;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                }
            }
        }
    }
}
#endif
