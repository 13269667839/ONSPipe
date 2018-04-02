#ifndef BasicSocket_hpp
#define BasicSocket_hpp

#include "SocketConfig.hpp"

class BasicSocket
{
public:
    BasicSocket(int _domain,SocketType _socktype, AddressFamily _family);
    ~BasicSocket();

    BasicSocket(int _socketfd,sockaddr_storage *addr,SocketType _socktype);

public:
    bool close();
    bool bind(std::string address, int port);

    int sockfd();
    bool setSocketOpt(int item,int opt,const void *val,socklen_t len);

    void initParams();

    bool initSocketfd();
    void initSockaddr(std::string &address, int &port);
public:
    int domain;
    SocketType socktype;
    AddressFamily addressFamily;
    // default 256
    long recvBuffSize;

    sockaddr_storage *sockaddrinfo;
private:
    // default -1
    int socketfd;
};

#endif