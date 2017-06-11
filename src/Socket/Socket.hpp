#ifndef Socket_hpp
#define Socket_hpp

#include <string>
#include <netdb.h>
#include <functional>

enum class SocketType : int
{
    TCP = 0,
    UDP
};
    
using socketFDIteration = std::function<bool(int,const addrinfo *)>;
    
class Socket
{
public:
    int socketfd;
    //default is 512
    int recvBuffSize;
private:
    SocketType type;
    addrinfo *addressInfo;
    addrinfo *currentAddrInfo;
public:
    Socket(std::string addr,int port,SocketType _type = SocketType::TCP);
    ~Socket();
    void close(int fd = -1);
    
    bool bind();
    //=== TCP === 
    bool listen();
    int accept();
    
    bool connect();
    
    ssize_t send(std::string buf,int fd = -1);
    void * receive(int fd = -1);
    
    //=== UDP ===
    ssize_t sendto(std::string buf);
    void * receiveFrom();
    
    void sendAll(std::string buf,int fd = -1);
    
    void setSocketOpt(int item,int opt,const void *val,socklen_t len,int fd = -1);
private:
    void setAddressInfo(std::string address,const char *port);
    void setSocketFileDescription(socketFDIteration iter);
    void * get_in_addr(sockaddr *sa);
};

#endif