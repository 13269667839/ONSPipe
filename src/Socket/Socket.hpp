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
    Socket();
    Socket(std::string addr,int port,SocketType _type = SocketType::TCP);
    ~Socket();
    void close(int fd = -1);
    
    bool bind();
    //=== TCP === 
    bool listen();
    int accept();
    
    bool connect();
    
    ssize_t send(void *buf,size_t len,int fd = -1);
    
    std::tuple<void *,long> receive(int fd = -1);
    
    //=== UDP ===
    ssize_t sendto(std::string buf);
    void * receiveFrom();
    
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

    //=== ssl ===
    void ssl_config();
    void ssl_close();
    ssize_t ssl_send(void *buf,size_t len);
    std::tuple<void *,long> ssl_read();
private:
    void setAddressInfo(std::string address,const char *port);
    void setSocketFileDescription(socketFDIteration iter);
    void * get_in_addr(sockaddr *sa);
    void initParam();
};

#endif
