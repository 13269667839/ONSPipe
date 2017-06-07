#ifndef HTTP_hpp
#define HTTP_hpp

#include <string>
#include <map>
#include "URL.hpp"
#include "HTTPResponse.hpp"
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
    std::map<std::string,std::string> *header;
public:
    HTTP(std::string _url,HTTPMethod _method = HTTPMethod::GET);
    ~HTTP();
    
    HTTPResponse * request();
    std::string rawHTTPStr();
    void addRequestHeader(std::string key,std::string val);
private:
    // === combine request msg ===
    std::string requestLine();
    std::string requestHead();
    std::string requestBody();
    
    // === parse response msg ===
    HTTPResponse * parseResponseLine(std::string &buf,std::string::size_type idx);
    std::map<std::string,std::string> * parseResponseHead(std::string &buf,std::string::size_type idx);
    
    void setDefaultHeader();
    void setSocketConfig(Socket &socket);
};

#endif
