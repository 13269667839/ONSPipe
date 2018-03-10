#include "Socket.hpp"
#include <unistd.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include "../Utility/Util.hpp"

#ifdef DEBUG
    #include <iostream>
#endif

void Socket::initParam()
{
    ctx = nullptr;
    ssl = nullptr;
    socketfd = -1;
    addressInfo = nullptr;
    currentAddrInfo = nullptr;
    recvBuffSize = 512;
    type = SocketType::TCP;
}

void Socket::close(int fd)
{
    if (fd == -1)
    {
        if (socketfd != -1)
        {
            ::close(socketfd);
            socketfd = -1;
        }
    }
    else
    {
        ::close(fd);
    }
}

Socket::~Socket()
{
    ssl_close();
    close();
    
    if (addressInfo)
    {
        freeaddrinfo(addressInfo);
        addressInfo = nullptr;
    }
    
    if (currentAddrInfo)
    {
        //free by above
        currentAddrInfo = nullptr;
    }
}

void Socket::setAddressInfo(std::string address, const char *port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;//ignore ipv4 or ipv6
    hints.ai_socktype = type == SocketType::TCP ? SOCK_STREAM : SOCK_DGRAM;
    const char *addr = nullptr;
    if (address.empty())
    {
        hints.ai_flags = AI_PASSIVE; //127.0.0.1
    }
    else
    {
        addr = address.c_str();
    }

    /**
     *  DNS or server name query
     *  @param1 ip or domain name
     *  @param2 serve name or port name
     *  @param3 addr info that you customized
     *  @param4 liked list returns
     */
    int res = getaddrinfo(addr, port, &hints, &addressInfo);
    if (res != 0)
    {
        throwError("Error occur on getaddrinfo, reason " + std::string(gai_strerror(res)));
    }
}

void Socket::setSocketFileDescription(socketFDIteration iter)
{
    if (!addressInfo)
    {
        return;
    }

    if (type == SocketType::TCP && !iter)
    {
        return;
    }

    currentAddrInfo = addressInfo;
    while (currentAddrInfo)
    {
        auto _sockfd = socket(currentAddrInfo->ai_family, currentAddrInfo->ai_socktype, currentAddrInfo->ai_protocol);
        if (_sockfd != -1)
        {
            if (!iter || iter(_sockfd, currentAddrInfo))
            {
                socketfd = _sockfd;
                break;
            }
        }
        currentAddrInfo = currentAddrInfo->ai_next;
    }
}

bool Socket::bind()
{
    if (!addressInfo)
    {
        return false;
    }

    setSocketFileDescription([](int _sockfd, const addrinfo *addr) {
        auto res = ::bind(_sockfd, addr->ai_addr, addr->ai_addrlen) != -1;
#ifdef DEBUG
        if (res)
        {
            std::cout << "bind to " << Socket::netAddressToHostAddress(*addr->ai_addr) << std::endl;
        }
#endif
        return res;
    });

    return socketfd != -1;
}

#pragma mark -- General method
int Socket::sockAddrLen(sockaddr_storage addr)
{
    auto len = -1;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
    if (addr.ss_family == AF_INET)
    {
        len = ((sockaddr_in *)&addr)->sin_len;
    }
    else if (addr.ss_family == AF_INET6)
    {
        len = ((sockaddr_in6 *)&addr)->sin6_len;
    }
#elif defined(__linux__)
    
#endif

    return len;
}

std::string Socket::netAddressToHostAddress(sockaddr addr)
{
    auto hostFormatedAddr = std::string();
    if (addr.sa_family == AF_INET)
    {
        //ipv4
        char src[INET_ADDRSTRLEN];
        inet_ntop(addr.sa_family, &(((sockaddr_in *)&addr)->sin_addr), src, sizeof(src));
        hostFormatedAddr += src;
    }
    else if (addr.sa_family == AF_INET6)
    {
        //ipv6
        char src[INET6_ADDRSTRLEN];
        inet_ntop(addr.sa_family, &(((sockaddr_in6 *)&addr)->sin6_addr), src, sizeof(src));
        hostFormatedAddr += src;
    }
    return hostFormatedAddr;
}

int Socket::setSocketOpt(int item,int opt,const void *val,socklen_t len,int fd)
{
    return setsockopt(fd == -1?socketfd:fd, item, opt, val, len);
}

std::string Socket::byteOrder()
{
    union {
        short value;
        char  bytes[sizeof(short)];
    } buf;

    buf.value = 0x0102;

    auto order = std::string();

    if (buf.bytes[0] == 1 && buf.bytes[1] == 2)
    {
        order += "big endian";
    }
    else if (buf.bytes[0] == 2 && buf.bytes[1] == 1)
    {
        order += "little endian";
    }
    else
    {
        order += "unknown";
    }

    return order;
}

#pragma mark -- UDP
ssize_t Socket::sendto(void *buf,size_t len,sockaddr_storage addr)
{
    if (type == SocketType::TCP)
    {
        throwError("this function work at udp mode");
    }

    if (!buf || len == 0)
    {
        return 0;
    }

    if (socketfd == -1 && !currentAddrInfo)
    {
        setSocketFileDescription(nullptr);
        if (socketfd == -1 || !currentAddrInfo)
        {
            throwError("socket fd set error");
        }
    }

    auto addri = (sockaddr *)&addr;
    socklen_t addrlen = Socket::sockAddrLen(addr);
    auto sendedBytes = ::sendto(socketfd, buf, len, 0, addri, addrlen);
    
    if (sendedBytes < 0)
    {
        throwError("error occur on sendto,reason " + std::string(gai_strerror(errno)));
    }

    return sendedBytes;
}

std::tuple<std::basic_string<unsigned char>,sockaddr_storage> Socket::receiveFrom()
{
    if (type == SocketType::TCP)
    {
        throwError("this function work at udp mode");
    }

    if (socketfd == -1)
    {
        throwError("socket fd error");
    }

    sockaddr_storage clientAddr;
    socklen_t addr_len = sizeof(clientAddr);
    Util::byte buffer[recvBuffSize];
    auto recvBytes = recvfrom(socketfd, buffer, recvBuffSize, 0, (sockaddr *)&clientAddr, &addr_len);

    if (recvBytes < 0)
    {
        throwError("error occur on receiveFrom,reason " + std::string(gai_strerror(errno)));
    }

    return {std::basic_string<Util::byte>(buffer, recvBytes),clientAddr};
}

#pragma mark -- SSL
void Socket::ssl_config()
{
    if (socketfd < 0)
    {
        throwError("socket fd is invalid");
        return;
    }

    SSL_load_error_strings();
    SSL_library_init();

    ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err)
        {
            std::string msg = "init SSL CTX failed:" + std::string(err);
            throwError(msg);
        }
        return;
    }

    ssl = SSL_new(ctx);
    if (!ssl)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err)
        {
            std::string msg = "new SSL with created CTX failed:" + std::string(err);
            throwError(msg);
        }
        return;
    }

    if (SSL_set_fd(ssl, socketfd) == 0)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err)
        {
            std::string msg = "add SSL to tcp socket failed:" + std::string(err);
            throwError(msg);
        }
        return;
    }

    RAND_poll();
    while (RAND_status() == 0)
    {
        unsigned short rand_ret = rand() % 65536;
        RAND_seed(&rand_ret, sizeof(rand_ret));
    }

    if (SSL_connect(ssl) != 1)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err)
        {
            std::string msg = "SSL connection failed:" + std::string(err);
            throwError(msg);
        }
    }
}

void Socket::ssl_close()
{
    if (ssl)
    {   
        //shut down ssl (1:success,0:Unidirectional closed,-1:error)
        if (SSL_shutdown(ssl) == -1)
        {
            auto err = ERR_reason_error_string(ERR_get_error());
            if (err) 
            {
                std::string msg = "SSL shutdown failed:" + std::string(err);
                throwError(msg);
            }
            return;
        }

        SSL_free(ssl);
        ssl = nullptr;
    }

    if (ctx) 
    {
        SSL_CTX_free(ctx);
        ctx = nullptr;
    }

    ERR_free_strings();
}

ssize_t Socket::ssl_send(void *buf,size_t len)
{
    ssize_t bytes = -1;

    if (ssl && buf && len)
    {
        bytes = SSL_write(ssl, buf, len);
    }

    return bytes;
}

std::vector<unsigned char> Socket::ssl_read()
{
    Util::byte tmpBuf[recvBuffSize];
    long bytes = -1;

    if (ssl)
    {
        bytes = SSL_read(ssl, tmpBuf, recvBuffSize);
        if (bytes < 0)
        {
            throwError("ssl read error");
        }
    }

    return std::vector<Util::byte>(tmpBuf, tmpBuf + bytes);
}

#pragma mark -- TCP
bool Socket::listen()
{
    return socketfd != -1 && ::listen(socketfd,10) != -1;
}

ssize_t Socket::send(void *buf,size_t len,int fd)
{
    if (type == SocketType::UDP)
    {
        throwError("this function work at tcp mode");
    }
    
    ssize_t bytes = -1;
    if (!buf || len == 0)
    {
        return bytes;
    }
    
    auto _sockfd = fd == -1?socketfd:fd;
    bytes = ::send(_sockfd, buf, len, 0);
    if (bytes == -1)
    {
        throwError("some error occur at send function");
    }

    return bytes;
}

std::vector<unsigned char> Socket::receive(int fd)
{
    if (type == SocketType::UDP)
    {
        throwError("this function work at tcp mode");
    }

    auto sockfd = fd == -1 ? socketfd : fd;
    if (sockfd == -1)
    {
        throwError("invalid socket fd");
    }

    Util::byte tmpBuf[recvBuffSize];
    auto bytes = recv(sockfd, tmpBuf, recvBuffSize, 0);

    if (bytes < 0)
    {
        throwError("receive error : " + std::string(gai_strerror(errno)));
    }

    return std::vector<Util::byte>(tmpBuf,tmpBuf + bytes);
}

int Socket::accept()
{
    sockaddr_storage visitorAddr;
    socklen_t len = sizeof(visitorAddr);
    int new_fd = ::accept(socketfd, (sockaddr *)(&visitorAddr), &len);

    if (new_fd == -1)
    {
        throwError("error occur on accept(),reason " + std::string(gai_strerror(errno)));
    }

#ifdef DEBUG
    std::cout << "connect from " << Socket::netAddressToHostAddress(*((sockaddr *)&visitorAddr)) << std::endl;
#endif

    return new_fd;
}

bool Socket::connect()
{
    if (type == SocketType::UDP)
    {
        throwError("this function work at tcp mode");
    }

    if (!addressInfo)
    {
        return false;
    }

    setSocketFileDescription([](int _sockfd, const addrinfo *addr) {
        bool res = ::connect(_sockfd, addr->ai_addr, addr->ai_addrlen) != -1;
#ifdef DEBUG
        if (res)
        {
            std::cout << "connect to " << Socket::netAddressToHostAddress(*addr->ai_addr) << std::endl;
        }
#endif
        return res;
    });

    return socketfd != -1;
}