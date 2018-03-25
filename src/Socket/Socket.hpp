#ifndef Socket_hpp
#define Socket_hpp

#include "../Utility/Util.hpp"

#include <netdb.h>
#include <functional>
#include <tuple>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

enum class SocketType
{
    TCP,
    UDP
};
    
using socketFDIteration = std::function<bool(int,const addrinfo *)>;
    
class Socket
{
public:
    //default is -1
    int socketfd;
    //default is 512
    int recvBuffSize;

    SSL_CTX *ctx;
    SSL *ssl;
private:
    SocketType type;
    addrinfo *addressInfo;
    addrinfo *currentAddrInfo;
public:
    Socket()
    {
        initParam();
    }

    Socket(std::string addr,int port,SocketType _type = SocketType::TCP) : Socket()
    {
        if (port > 0)
        {
            type = _type;
            addressInfo = setAddressInfo(addr, port,AF_UNSPEC);
        }
    }

    ~Socket();
    void close(int fd = -1);
    
    bool bind();
#pragma mark -- TCP
    bool listen();
    int accept();
    
    bool connect();
    
    ssize_t send(void *buf,size_t len,int fd = -1);
    
    std::vector<unsigned char> receive(int fd = -1);
#pragma mark -- UDP
    ssize_t sendto(void *buf,size_t len,sockaddr_storage addr);
    std::tuple<std::basic_string<unsigned char>,sockaddr_storage> receiveFrom();
#pragma mark -- General method
    static std::string netAddressToHostAddress(sockaddr addr);

    /**
     * test machine byte order
     * return string
     */
    static std::string byteOrder();

    template <typename BufferType,typename BufferLength>
    void sendAll(BufferType buffer,BufferLength length,bool ssl,int fd = -1)
    {
        BufferLength sendBytes = 0;
        while (sendBytes < length) 
        {
            if (type == SocketType::TCP)
            {
                if (ssl)
                {
                    auto _sendBytes = ssl_send(&buffer[sendBytes],length - sendBytes);
                    sendBytes = _sendBytes;
                }
                else 
                {
                    auto _sendBytes = send(&buffer[sendBytes],length - sendBytes,fd);
                    sendBytes = _sendBytes;
                }
            }
            else if (type == SocketType::UDP)
            {

            }
        }
    }
    
    int setSocketOpt(int item,int opt,const void *val,socklen_t len,int fd = -1);

#pragma mark -- ssl
    //target 0:client 1:server
    void ssl_config(int target);
    void ssl_close();
    ssize_t ssl_send(void *buf,size_t len);
    std::vector<unsigned char> ssl_read();
    void ssl_set_fd(int fd);
    void ssl_connect();
#ifdef DEBUG
    void ssl_certification_info();
#endif
private:
    addrinfo * setAddressInfo(std::string address,int port,int family);
    void setSocketFileDescription(socketFDIteration iter);
    void initParam();
};

#endif
