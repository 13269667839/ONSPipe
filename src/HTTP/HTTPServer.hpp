#ifndef HTTPServer_hpp
#define HTTPServer_hpp

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "../Utility/HTTPReqMsgParser.hpp"
#include "../Socket/TCPSocket.hpp"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #define Kqueue
#elif defined(__linux__)
    #define Epoll
#endif

using LoopCallback = std::function<void (HTTPRequest &request,HTTPResponse &response)>;

class HTTPServer
{
public:
    HTTPServer(int port);
    ~HTTPServer();
    
    void loop(LoopCallback callback);
private:
    int port;
    TCPSocket *sock;
    int listenfd;
    std::map<int,std::shared_ptr<TCPSocket>> *dict;
private:
    void setSocket();
    void disconnect(int sockfd);
    
#ifdef Kqueue
    void kqueueLoop(const LoopCallback &callback);
    int setupKqueue();
    void kqueueAccept(long count, int kq);
    void deleteEvent(int sockfd, int kq, int eventType);
    int kqueueParseRecvRequest(HTTPRequest &request, HTTPReqMsgParser &parser, long totalLength, int sockfd);
#elif defined(Epoll)
    void epollLoop(const LoopCallback &callback);
#endif
};

#endif 
