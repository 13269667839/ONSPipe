#ifndef Response_hpp
#define Response_hpp

#include <string>
#include <ostream>
#include <map>

class HTTPResponse
{
public:
    HTTPResponse();
    ~HTTPResponse();
    friend std::ostream & operator << (std::ostream &os,HTTPResponse *res);
    friend std::ostream & operator << (std::ostream &os,HTTPResponse &res);
    
    void initParameter();
    
    std::string toResponseMessage();
    
    void addResponseHead(std::string key,std::string value);
    void setResponseLine(std::string version,int statusCode,std::string reason);
public:
    std::string version;
    int statusCode;
    std::string reason;
    std::map<std::string,std::string> *header;
    std::basic_string<unsigned char> responseBody;
};

#endif
