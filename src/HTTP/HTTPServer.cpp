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
    dict = new std::map<int, std::shared_ptr<TCPSocket>>();
}

HTTPServer::~HTTPServer()
{
    if (sock)
    {
        delete sock;
        sock = nullptr;
    }

    if (dict)
    {
        STLExtern::releaseMap(dict);
        delete dict;
        dict = nullptr;
    }
}

void HTTPServer::disconnect(int sockfd)
{
    auto iter = dict->find(sockfd);
    if (iter == dict->end())
    {
        return;
    }

    auto client = iter->second;
    dict->erase(sockfd);
    if (client != nullptr)
    {
        client.reset();
    }
}

void HTTPServer::setSocket()
{
    auto address = std::string();
    auto family = AddressFamily::IPV4;

    auto addressList = SocketConfig::getAddressInfo(address, std::to_string(port), family, SocketType::TCP);
    if (!addressList)
    {
        throwError("getAddressInfo() return null");
        return;
    }

    auto domain = addressList->ai_family;
    freeaddrinfo(addressList);
    addressList = nullptr;

    sock = new TCPSocket(domain, family);

    int yes = 1;
    if (!sock->setSocketOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
    {
        throwError("set SO_REUSEADDR flag error");
        return;
    }

    if (!sock->bind(address, port))
    {
        throwError("bind() error " + std::string(gai_strerror(errno)));
        return;
    }

    if (!sock->listen())
    {
        throwError("listen() error " + std::string(gai_strerror(errno)));
    }

    listenfd = sock->sockfd();
    if (!SocketConfig::setNonBlocking(listenfd))
    {
        throwError("set none blocking error : " + std::string(gai_strerror(errno)));
    }

#ifdef __APPLE__
    yes = 1;
    if (!sock->setSocketOpt(SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)))
    {
        throwError("set SO_NOSIGPIPE flag error");
        return;
    }
#endif
}

void HTTPServer::loop(LoopCallback callback)
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
        auto client = sock->accept();
        if (!client)
        {
            continue;
        }

        auto clientfd = client->sockfd();
        if (clientfd <= 0)
        {
            continue;
        }

        if (!SocketConfig::setNonBlocking(clientfd))
        {
            throwError("set none blocking error : " + std::string(gai_strerror(errno)));
        }

        struct kevent changelist[2];
        EV_SET(&changelist[0], clientfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        EV_SET(&changelist[1], clientfd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        kevent(kq, changelist, 2, nullptr, 0, nullptr);

        dict->insert(std::make_pair(clientfd, client));
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

    auto iter = dict->find(sockfd);
    if (iter == dict->end() || iter->second == nullptr)
    {
        return 2;
    }
    auto client = iter->second;

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
            auto buff = client->receive();
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

void HTTPServer::kqueueLoop(const LoopCallback &callback)
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
                if (status == 0)
                {
                    response.initParameter();
                    callback(request, response);
                }
                else if (status == 1 && status == 2)
                {
                    deleteEvent(sockfd, kq, EVFILT_READ);
                    if (i + 1 == activeEventCount)
                    {
                        deleteEvent(sockfd, kq, EVFILT_WRITE);
                        disconnect(sockfd);
                    }
                }
            }
            else if (event.filter == EVFILT_WRITE) //write event
            {
                auto iter = dict->find(sockfd);
                if (iter == dict->end() || iter->second == nullptr)
                {
                    deleteEvent(sockfd, kq, EVFILT_WRITE);
                    if (i + 1 == activeEventCount)
                    {
                        disconnect(sockfd);
                    }
                    continue;
                }

                try
                {
                    auto client = iter->second;
                    auto message = response.toResponseMessage();
                    client->sendAll(const_cast<char *>(message.c_str()), message.size());
                }
                catch (std::logic_error error)
                {
                    auto code = errno;
                    if (code != 32)
                    {
                        deleteEvent(sockfd, kq, EVFILT_READ);
                        deleteEvent(sockfd, kq, EVFILT_WRITE);
                        disconnect(sockfd);
                    }
                    continue;
                }

                if (request.header->find("Connection")->second == "Close")
                {
                    deleteEvent(sockfd, kq, EVFILT_READ);
                    deleteEvent(sockfd, kq, EVFILT_WRITE);
                    disconnect(sockfd);
                    break;
                }
            }
        }
    }
}
#endif

#pragma mark-- Epoll
#ifdef Epoll
void HTTPServer::epollLoop(const LoopCallback &callback)
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
