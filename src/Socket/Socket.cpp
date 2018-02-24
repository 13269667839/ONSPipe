#include "Socket.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include "../Utility/Util.hpp"

Socket::Socket(std::string addr,int port,SocketType _type)
{
    ctx = nullptr;
    ssl = nullptr;
    socketfd = -1;
    addressInfo = nullptr;
    currentAddrInfo = nullptr;
    recvBuffSize = 512;
    if (port > 0)
    {
        type = _type;
        setAddressInfo(addr, std::to_string(port).c_str());
    }
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
        //free by above (line 43)
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

std::tuple<void *,long> Socket::receive(int fd)
{
    if (type == SocketType::UDP)
    {
        throwError("this function work at tcp mode");
    }
    
    void *recvBuf = nullptr;
    int sockfd = fd == -1?socketfd:fd;
    long bytes = -2;
    if (sockfd != -1)
    {
        Util::byte tmpBuf[recvBuffSize];
        bytes = recv(sockfd, tmpBuf, recvBuffSize, 0);
        if (bytes > 0)
        {
            recvBuf = new Util::byte[bytes]();
            memcpy(recvBuf, tmpBuf, bytes);
        }
        else if (bytes == -1)
        {
            auto err = "receive error : " + std::string(gai_strerror(errno));
            throwError(err);
        }
#ifdef DEBUG
        else if (bytes == 0)
        {
            std::cout<<"connect is closed"<<std::endl;
        }
#endif
    }
    
    return std::make_tuple(recvBuf,bytes);
}

ssize_t Socket::sendto(std::string buf)
{
    if (type == SocketType::TCP)
    {
        throwError("this function work at udp mode");
    }
    
    ssize_t bytes = -1;
    if (!buf.empty())
    {
        setSocketFileDescription([](int fd,const addrinfo *info) { return info != nullptr; });
        if (socketfd != -1 && currentAddrInfo)
        {
            bytes = ::sendto(socketfd,buf.c_str(),buf.size(),0,currentAddrInfo->ai_addr,currentAddrInfo->ai_addrlen);
            if (bytes == -1)
            {
                std::string err = "receive error : " + std::string(gai_strerror(errno));
                throwError(err);
            }
        }
    }
    return bytes;
}

void * Socket::receiveFrom()
{
    if (type == SocketType::TCP)
    {
        throwError("this function work at udp mode");
    }
    
    void *recvBuf = nullptr;
    if (socketfd != -1)
    {
        sockaddr_storage their_addr;
        socklen_t addr_len = sizeof(their_addr);
        int16_t buf[recvBuffSize];
        auto bytes = recvfrom(socketfd, buf, recvBuffSize, 0, (sockaddr *)&their_addr, &addr_len);
        if (bytes > 0)
        {
            recvBuf = new int16_t[bytes]();
            memcpy(recvBuf, buf, bytes);
        }
        else if (bytes == -1)
        {
            std::string err = "receive error : " + std::string(gai_strerror(errno));
            throwError(err);
        }
    }
    
    return recvBuf;
}

int Socket::setSocketOpt(int item,int opt,const void *val,socklen_t len,int fd)
{
    return setsockopt(fd == -1?socketfd:fd, item, opt, val, len);
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
        SSL_write(ssl, buf, len);
    }

    return bytes;
}

std::tuple<void *,long> Socket::ssl_read()
{    
    void *recvBuf = nullptr;
    long bytes = -2;

    if (ssl)
    {
        Util::byte tmpBuf[recvBuffSize];
        bytes = SSL_read(ssl,tmpBuf,recvBuffSize);
        if (bytes > 0)
        {
            recvBuf = new Util::byte[bytes]();
            memcpy(recvBuf, tmpBuf, bytes);
        }
    }
    
    return std::make_tuple(recvBuf,bytes);
}