#ifndef HTTPClient_hpp
#define HTTPClient_hpp

#include "URL.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "../Socket/TCPSocket.hpp"

enum class HTTPMethod
{
    GET = 0,
    POST,
    HEAD
};

/*
ssl client procedure

meth = SSLv23_client_method();
ctx = SSL_CTX_new (meth); 
ssl = SSL_new(ctx);
fd = socket();
connect();
SSL_set_fd(ssl,fd);
SSL_connect(ssl); 
SSL_write(ssl,"Hello world",strlen("Hello World!"));
*/ 
    
class HTTPClient
{
public:
    void setRequestHeader(std::string key,std::string value);
    
    std::unique_ptr<HTTPResponse> request();

    //default is 10
    long timeoutSeconds;
public:
    HTTPClient(std::string _url,HTTPMethod _method = HTTPMethod::GET);
    ~HTTPClient();
private:
    void setHttpRequest();
    std::string methodStr();

    std::unique_ptr<HTTPResponse> requestByTCPSocket();

    void sendMsg(TCPSocket &socket);
    std::unique_ptr<HTTPResponse> recvMsg(TCPSocket &socket);

    void setSocketConfig(TCPSocket &socket);
private:
    HTTPMethod method;
    URL *url;
    HTTPRequest *httpRequest;
};

#endif
