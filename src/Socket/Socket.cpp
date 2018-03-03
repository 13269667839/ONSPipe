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

void Socket::setAddressInfo(std::string address,const char *port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;//ignore ipv4 or ipv6
    hints.ai_socktype = type == SocketType::TCP?SOCK_STREAM:SOCK_DGRAM;
    const char *addr = nullptr;
    if (address.empty())
    {
        hints.ai_flags = AI_PASSIVE;//127.0.0.1
    }
    else
    {
        addr = address.c_str();
    }
    
    int res = getaddrinfo(addr, port, &hints, &addressInfo);
    if (res != 0)
    {
        std::string errMsg = "getaddrinfo: " + std::string(gai_strerror(res));
        throwError(errMsg.c_str());
    }
}

void Socket::setSocketFileDescription(socketFDIteration iter)
{
    if (!addressInfo || !iter)
    {
        return;
    }
    
    currentAddrInfo = addressInfo;
    while (currentAddrInfo)
    {
        int _sockfd = socket(currentAddrInfo->ai_family, currentAddrInfo->ai_socktype, currentAddrInfo->ai_protocol);
        if (_sockfd != -1 && iter(_sockfd,currentAddrInfo))
        {
            socketfd = _sockfd;
            break;
        }
        currentAddrInfo = currentAddrInfo->ai_next;
    }
}

//address of human readable format
void * Socket::get_in_addr(sockaddr *sa)
{
    void *res = nullptr;
    if (sa->sa_family == AF_INET)
    {
        res = &(((sockaddr_in *)sa)->sin_addr);
    }
    else if (sa->sa_family == AF_INET6)
    {
        res = &(((sockaddr_in6 *)sa)->sin6_addr);
    }
    return res;
}

bool Socket::bind()
{
    if (addressInfo)
    {
        auto self = this;
        setSocketFileDescription([&self](int _sockfd,const addrinfo *addr)
        {
            bool res = false;
            if (::bind(_sockfd,addr->ai_addr,addr->ai_addrlen) != -1)
            {
                res = true;
#ifdef DEBUG
                char s[INET6_ADDRSTRLEN];
                inet_ntop(addr->ai_family,self->get_in_addr((sockaddr *)addr->ai_addr),s,sizeof(s));
                std::cout<<"bind to "<<s<<std::endl;
#endif
            }
            return res;
        });
    }
    return socketfd != -1;
}

bool Socket::listen()
{
    return socketfd != -1 && ::listen(socketfd,10) != -1;
}

int Socket::accept()
{
    sockaddr_storage visitorAddr;
    socklen_t len = sizeof(visitorAddr);
    int new_fd = ::accept(socketfd, (sockaddr *)(&visitorAddr), &len);

    if (new_fd == -1)
    {
        throwError("some error occur at accept function");
    }
#ifdef DEBUG
    else
    {
        char s[INET6_ADDRSTRLEN];
        inet_ntop(visitorAddr.ss_family,get_in_addr((sockaddr *)&visitorAddr),s,sizeof(s));
        std::cout<<"connect from "<<s<<std::endl;
    }
#endif

    return new_fd;
}

bool Socket::connect()
{
    if (type == SocketType::UDP)
    {
        throwError("this function work at tcp mode");
    }
    
    if (addressInfo)
    {
        auto self = this;
        setSocketFileDescription([&self](int _sockfd,const addrinfo *addr)
        {
            bool res = false;
            if (::connect(_sockfd,addr->ai_addr,addr->ai_addrlen) != -1)
            {
                res = true;
#ifdef DEBUG
                char s[INET6_ADDRSTRLEN];
                inet_ntop(addr->ai_family,self->get_in_addr((sockaddr *)addr->ai_addr),s,sizeof(s));
                std::cout<<"connect to "<<s<<std::endl;
#endif
            }
            return res;
        });
    }
    
    return socketfd != -1;
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

int Socket::setSocketOpt(int item,int opt,const void *val,socklen_t len,int fd)
{
    return setsockopt(fd == -1?socketfd:fd, item, opt, val, len);
}

#pragma mark -- UDP
ssize_t Socket::sendto(void *buf,size_t len,sockaddr_in *addr)
{
    if (type == SocketType::TCP)
    {
        throwError("this function work at udp mode");
    }

    if (!buf || len == 0)
    {
        return -1;
    }

    if (socketfd == -1 && !currentAddrInfo)
    {
        setSocketFileDescription([](int fd, const addrinfo *info) {
            auto res = info != nullptr;
#ifdef DEBUG
            if (res)
            {
                std::cout << "socket file description = " << fd << std::endl;
            }
#endif
            return res;
        });

        if (socketfd == -1 || !currentAddrInfo)
        {
            throwError("socket fd set error");
        }
    }

    ssize_t bytes = -1;

    if (addr)
    {
        bytes = ::sendto(socketfd, buf, len, 0, (sockaddr *)addr, sizeof(*addr));
    }
    else 
    {
        bytes = ::sendto(socketfd, buf, len, 0, currentAddrInfo->ai_addr, currentAddrInfo->ai_addrlen);
    }

    return bytes;
}

std::tuple<std::basic_string<unsigned char>,sockaddr_in> Socket::receiveFrom()
{
    if (type == SocketType::TCP)
    {
        throwError("this function work at udp mode");
    }

    if (socketfd == -1)
    {
        throwError("socket fd error");
    }

    sockaddr_in clientAddr;
    socklen_t addr_len = sizeof(clientAddr);
    Util::byte buffer[recvBuffSize];
    auto recvBytes = recvfrom(socketfd, buffer, recvBuffSize, 0, (sockaddr *)&clientAddr, &addr_len);

    if (recvBytes < 0)
    {
        throwError("receive error : " + std::string(gai_strerror(errno)));
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