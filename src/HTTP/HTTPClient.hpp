#ifndef HTTPClient_hpp
#define HTTPClient_hpp

#include "URL.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "../Socket/TCPSocket.hpp"
#include "../Socket/BIOSocket.hpp"

enum class HTTPMethod
{
    GET = 0,
    POST,
    HEAD
};
    
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
    std::string setSocketParams();
    void setSocketOptions();
    void sendMsg();
    std::unique_ptr<HTTPResponse> recvMsg();
    void closeConnection();

    std::unique_ptr<HTTPResponse> requestViaSSL();
    
private:
    HTTPMethod method;
    URL *url;
    HTTPRequest *httpRequest;

    TCPSocket *socket;
    bool isConnect;

    BIOSocket *socketViaSSL;
    bool isViaSSL;
};

#endif
