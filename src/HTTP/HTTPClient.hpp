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

using RequestCallback = std::function<void (HTTPResponse *res,std::string err)>;
    
class HTTPClient
{
public:
    void setRequestHeader(std::string key,std::string value);
    
    HTTPResponse * syncRequest();
    void asyncRequest(RequestCallback callback);
public:
    HTTPClient(std::string _url,HTTPMethod _method = HTTPMethod::GET);
    ~HTTPClient();
private:
    void setHttpRequest();
    void setSocketConfig(Socket &socket);
private:
    HTTPMethod method;
    URL *url;
    HTTPRequest *httpRequest;
};

#endif
