#ifndef HTTPServer_hpp
#define HTTPServer_hpp

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "../Socket/Socket.hpp"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #define Kqueue
#elif defined(__linux__)
    #define Epoll
#endif

using RunAndLoopCallback = std::function<void (HTTPRequest &request,HTTPResponse &response)>;

class HTTPServer
{
public:
    HTTPServer(int port);
    ~HTTPServer();
    
    void runAndLoop(RunAndLoopCallback callback);
private:
    int port;
    Socket *sock;
private:
    void setSocket();
    
#ifdef Kqueue
    void kqueueLoop(RunAndLoopCallback &callback);
    std::tuple<int,int> setupKqueue();
    void kqueueAccept(long count,int kq);
    void kqueueError(int sockfd,int kq);
    void kqueueSend(int sockfd,HTTPRequest &request,HTTPResponse &response,RunAndLoopCallback &callback);
#elif defined(Epoll)
    void epollLoop(const RunAndLoopCallback &callback);
#endif
};

#endif 
