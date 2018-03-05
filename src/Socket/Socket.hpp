#ifndef Socket_hpp
#define Socket_hpp

#include <string>
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
            setAddressInfo(addr, std::to_string(port).c_str());
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
    ssize_t sendto(void *buf,size_t len,sockaddr_in *addr);
    std::tuple<std::basic_string<unsigned char>,sockaddr_in> receiveFrom();
#pragma mark -- General method
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
    void ssl_config();
    void ssl_close();
    ssize_t ssl_send(void *buf,size_t len);
    std::vector<unsigned char> ssl_read();
private:
    void setAddressInfo(std::string address,const char *port);
    void setSocketFileDescription(socketFDIteration iter);
    void initParam();
};

#endif
