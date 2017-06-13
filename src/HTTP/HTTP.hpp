#ifndef HTTP_hpp
#define HTTP_hpp

#include "URL.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "../Socket/Socket.hpp"

enum class HTTPMethod : int
{
    GET = 0,
    POST
};

class HTTP
{
public:
    HTTPMethod method;
    URL *url;
    HTTPRequest *httpRequest;
public:
    HTTP(std::string _url,HTTPMethod _method = HTTPMethod::GET);
    ~HTTP();
    
    HTTPResponse * sendRequest();
private:
    void setHttpRequest();
    void setSocketConfig(Socket &socket);
};

#endif
