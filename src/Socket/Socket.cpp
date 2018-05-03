#include "Socket.hpp"
#include <unistd.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <cstring>

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
            std::cout << "bind to " << SocketConfig::netAddressToHostAddress(*addr->ai_addr) << std::endl;
        }
#endif
        return res;
    });

    return socketfd != -1;
}

#pragma mark -- General method
int Socket::setSocketOpt(int item,int opt,const void *val,socklen_t len,int fd)
{
    return setsockopt(fd == -1?socketfd:fd, item, opt, val, len);
}

#pragma mark -- UDP
ssize_t Socket::sendto(void *buf, size_t len, sockaddr_storage addr)
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
    auto addrlen = -1;

    if (addr.ss_family == AF_INET)
    {
        addrlen = sizeof(sockaddr_in);
    }
    else if (addr.ss_family == AF_INET6)
    {
        addrlen = sizeof(sockaddr_in6);
    }

    if (addrlen == -1)
    {
        throwError("address len is -1");
    }

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

    return std::make_tuple(std::basic_string<Util::byte>(buffer, recvBytes),clientAddr);
}

#pragma mark -- SSL
void Socket::ssl_config(int target)
{
    SSL_load_error_strings();
    SSL_library_init();

    const SSL_METHOD *sslMethod = nullptr;
    if (target == 0) 
    {
        sslMethod = SSLv23_client_method();
    }

    if (!sslMethod) 
    {
        throwError("ssl method is null");
    }

    ctx = SSL_CTX_new(sslMethod);
    if (!ctx)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err)
        {
            throwError("init SSL CTX failed:" + std::string(err));
        }
        return;
    }

    ssl = SSL_new(ctx);
    if (!ssl)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err)
        {
            throwError("new SSL with created CTX failed:" + std::string(err));
        }
    }
}

void Socket::ssl_connect()
{
    if (!ssl) 
    {
        throwError("ssl connect params invalid");
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
            throwError("SSL connection failed:" + std::string(err));
        }
    }
}

void Socket::ssl_set_fd(int fd)
{
    if (!ssl || fd < 0) 
    {
        throwError("ssl_set_fd params invalid");
    }

    auto res = SSL_set_fd(ssl,fd);
    if (res == 0)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err) 
        {
            throwError("add SSL to tcp socket failed:" + std::string(err));
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
                throwError("SSL shutdown failed:" + std::string(err));
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

#ifdef DEBUG
void Socket::ssl_certification_info()
{
    if (!ssl)
    {
        return;
    }

    X509 *cert = SSL_get_peer_certificate(ssl);

    if (!cert)
    {
        std::cout << "无证书信息" << std::endl;
        return;
    }

    auto str = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    if (str)
    {
        std::cout << "证书 : " << str << std::endl;
        delete str;
        str = nullptr;
    }

    str = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    if (str)
    {
        std::cout << "颁发者 : " << str << std::endl;
        delete str;
        str = nullptr;
    }

    X509_free(cert);
}
#endif

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
    std::cout << "connect from " << SocketConfig::netAddressToHostAddress(*((sockaddr *)&visitorAddr)) << std::endl;
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
            std::cout << "connect to " << SocketConfig::netAddressToHostAddress(*addr->ai_addr) << std::endl;
        }
#endif
        return res;
    });

    return socketfd != -1;
}