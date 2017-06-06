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
    friend std::ostream & operator << (std::ostream &os,HTTPResponse res);
    
    void parseResponseLine(std::string line);
    void parseResponseHead(std::string head);
    void parseResponseBody(std::string body);
    
    std::string toResponseMessage();
public:
    //line
    std::string httpVersion;
    int statusCode;
    std::string reason;
    
    std::map<std::string,std::string> *header;
    std::string responseBody;
};

#endif
