#ifndef BasicSocket_hpp
#define BasicSocket_hpp

#include "SocketConfig.hpp"

class BasicSocket
{
public:
    BasicSocket(int _domain,SocketType _socktype, AddressFamily _family);
    ~BasicSocket();

    bool close();
    bool bind(std::string address, int port);

    int sockfd();
private:
    bool initSocketfd();
    void initSockaddr(std::string &address, int &port);
public:
    int domain;
    SocketType socktype;
    AddressFamily addressFamily;
    // default 256
    long recvBuffSize;
private:
    // default -1
    int socketfd;
    sockaddr_storage *sockaddrinfo;
};

#endif