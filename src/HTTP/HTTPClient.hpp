#ifndef HTTPClient_hpp
#define HTTPClient_hpp

#include "URL.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "../Socket/Socket.hpp"
#include <memory>

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
    void setSocketConfig(Socket &socket);
    std::string methodStr();

    void checkParams();
    void sendMsg(Socket &socket);
    HTTPResponse * recvMsg(Socket &socket);
private:
    HTTPMethod method;
    URL *url;
    HTTPRequest *httpRequest;
    bool https;
};

#endif
