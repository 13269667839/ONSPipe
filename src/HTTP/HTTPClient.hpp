#ifndef HTTPClient_hpp
#define HTTPClient_hpp

#include "URL.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "../Socket/Socket.hpp"

enum class HTTPMethod : int
{
    GET = 0,
    POST
};

class HTTPClient
{
public:
    HTTPMethod method;
    URL *url;
    HTTPRequest *httpRequest;
public:
    HTTPClient(std::string _url,HTTPMethod _method = HTTPMethod::GET);
    ~HTTPClient();
    
    HTTPResponse * sendRequest();
private:
    void setHttpRequest();
    void setSocketConfig(Socket &socket);
};

#endif
