#include "BasicSocket.hpp"
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>

BasicSocket::BasicSocket(int _domain,SocketType _socktype, AddressFamily _family)
{
    initParams();

    domain = _domain;
    socktype = _socktype;
    addressFamily = _family;

    if (addressFamily == AddressFamily::Default)
    {
        throwError("not implement");
    }

    if (!initSocketfd())
    {
        throwError("socket() error " + std::string(gai_strerror(errno)));
    }
}

BasicSocket::~BasicSocket()
{
    close();

    if (sockaddrinfo) 
    {
        delete sockaddrinfo;
        sockaddrinfo = nullptr;
    }
}

BasicSocket::BasicSocket(int _socketfd, sockaddr_storage *addr, SocketType _socktype)
{
    initParams();

    if (_socketfd == -1)
    {
        throwError("socket fd is -1");
        return;
    }

    socketfd = _socketfd;

    if (!addr)
    {
        throwError("address is nullptr");
        return;
    }

    sockaddrinfo = addr;

    if (addr->ss_family == AF_INET)
    {
        addressFamily = AddressFamily::IPV4;
    }
    else if (addr->ss_family == AF_INET6)
    {
        addressFamily = AddressFamily::IPV6;
    }

    socktype = _socktype;
}

void BasicSocket::initParams()
{
    recvBuffSize = 256;
    sockaddrinfo = nullptr;
    socketfd = -1;

    domain = 0;
    socktype = SocketType::TCP;
    addressFamily = AddressFamily::IPV4;
}

bool BasicSocket::initSocketfd()
{
    auto type = SocketConfig::socketTypeRawValue(socktype);

    auto _socketfd = socket(domain, type, 0);

    if (_socketfd != -1)
    {
        socketfd = _socketfd;
    }

    return _socketfd != -1;
}

bool BasicSocket::close()
{
    if (socketfd == -1)
    {
        return true;
    }

    auto res = ::close(socketfd) == 0;
    socketfd = -1;

    return res;
}

bool BasicSocket::bind(std::string address, int port)
{
    if (socketfd == -1)
    {
        throwError("sock fd invalid");
        return false;
    }

    if (!sockaddrinfo)
    {
        initSockaddr(address, port);
    }
    
    if (!sockaddrinfo)
    {
        throwError("sock addr info is null");
        return false;
    }

    auto len = SocketConfig::addressLen(*sockaddrinfo);
    auto naddr = reinterpret_cast<sockaddr *>(sockaddrinfo);

    return ::bind(socketfd, naddr, len) == 0;
}

void BasicSocket::initSockaddr(std::string &address, int &port)
{
    if (addressFamily == AddressFamily::IPV4)
    {
        auto addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));

        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
        inet_pton(AF_INET,address.empty() ? "127.0.0.1" : address.c_str(),&addr->sin_addr);

        sockaddrinfo = reinterpret_cast<sockaddr_storage *>(addr);
    }
    else if (addressFamily == AddressFamily::IPV6)
    {
        auto addr = (sockaddr_in6 *)malloc(sizeof(sockaddr_in6));

        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(port);
        inet_pton(AF_INET6, address.empty() ? "::1" : address.c_str(), &addr->sin6_addr);

        sockaddrinfo = reinterpret_cast<sockaddr_storage *>(addr);
    }
}

int BasicSocket::sockfd()
{
    return socketfd;
}

bool BasicSocket::setSocketOpt(int item, int opt, const void *val, socklen_t len)
{
    return setsockopt(socketfd, item, opt, val, len) == 0;
}