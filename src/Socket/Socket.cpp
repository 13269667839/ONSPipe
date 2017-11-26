#include "Socket.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../Utility/Util.hpp"

Socket::Socket(std::string addr,int port,SocketType _type)
{
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
        Util::throwError(errMsg.c_str());
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
                char s[INET6_ADDRSTRLEN];
                inet_ntop(addr->ai_family,self->get_in_addr((sockaddr *)addr->ai_addr),s,sizeof(s));
                std::cout<<"bind to "<<s<<std::endl;
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
        Util::throwError("some error occur at accept function");
    }
    else
    {
        char s[INET6_ADDRSTRLEN];
        inet_ntop(visitorAddr.ss_family,get_in_addr((sockaddr *)&visitorAddr),s,sizeof(s));
        std::cout<<"connect from "<<s<<std::endl;
    }

    return new_fd;
}

bool Socket::connect()
{
    if (type == SocketType::UDP)
    {
        Util::throwError("this function work at tcp mode");
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
                char s[INET6_ADDRSTRLEN];
                inet_ntop(addr->ai_family,self->get_in_addr((sockaddr *)addr->ai_addr),s,sizeof(s));
                std::cout<<"connect to "<<s<<std::endl;
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
        Util::throwError("this function work at tcp mode");
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
        Util::throwError("some error occur at send function");
    }

    return bytes;
}

std::tuple<void *,long> Socket::receive(int fd)
{
    if (type == SocketType::UDP)
    {
        Util::throwError("this function work at tcp mode");
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
            Util::throwError(err);
        }
        else if (bytes == 0)
        {
            std::cout<<"connect is closed"<<std::endl;
        }
    }
    
    return {recvBuf,bytes};
}

ssize_t Socket::sendto(std::string buf)
{
    if (type == SocketType::TCP)
    {
        Util::throwError("this function work at udp mode");
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
                Util::throwError(err);
            }
        }
    }
    return bytes;
}

void * Socket::receiveFrom()
{
    if (type == SocketType::TCP)
    {
        Util::throwError("this function work at udp mode");
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
            Util::throwError(err);
        }
    }
    
    return recvBuf;
}

int Socket::setSocketOpt(int item,int opt,const void *val,socklen_t len,int fd)
{
    return setsockopt(fd == -1?socketfd:fd, item, opt, val, len);
}

void Socket::sendAll(std::string buf,int fd)
{
    while (!buf.empty())
    {
        ssize_t send_bytes = 0;
        
        if (type == SocketType::TCP)
        {
            auto u8_str = const_cast<char *>(buf.c_str());
            send_bytes = send(u8_str,buf.size(),fd);
        }
        else if (type == SocketType::UDP)
        {
            Util::throwError("not implement yet!");
        }
        
        if (send_bytes <= buf.size())
        {
            buf = buf.substr(send_bytes);
        }
    }
}
