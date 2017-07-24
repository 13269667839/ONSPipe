#ifndef HTTPServer_hpp
#define HTTPServer_hpp

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "../Socket/Socket.hpp"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #define Kqueue
    #define MAX_EVENT_COUNT 64

    #include <sys/event.h>
    #include <iostream>
#elif defined(__linux__)
    #define Epoll
#else
    #define Select

    #include <sys/select.h>
#endif

using RunAndLoopCallback = std::function<void (const HTTPRequest &request,HTTPResponse &response)>;

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
    void mainLoop(const RunAndLoopCallback &callback);
    
#ifdef Kqueue
    void kqueueLoop(const RunAndLoopCallback &callback);
#elif defined(Select)
    void selectLoop(const RunAndLoopCallback &callback);
#endif
};

#endif 
