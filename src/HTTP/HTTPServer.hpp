#ifndef HTTPServer_hpp
#define HTTPServer_hpp

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "../Socket/Socket.hpp"
#include "../Utility/HTTPReqMsgParser.hpp"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #define Kqueue
    #include <set>
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

    int listenfd;
    #ifdef Kqueue
    std::set<int> fds;
    #endif
private:
    void setSocket();
    void disconnect(int sockfd);
    
#ifdef Kqueue
    void kqueueLoop(RunAndLoopCallback &callback);
    int setupKqueue();
    void kqueueAccept(long count, int kq);
    void kqueueError(int sockfd, int kq, int eventType);
    //0 : normal 1 : empty 2 : error
    int kqueueParseRecvRequest(HTTPRequest &request, HTTPReqMsgParser &parser, long totalLength, int sockfd);
#elif defined(Epoll)
    void epollLoop(const RunAndLoopCallback &callback);
#endif
};

#endif 
