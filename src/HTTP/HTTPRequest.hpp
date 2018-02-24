#ifndef HTTPRequest_hpp
#define HTTPRequest_hpp

#include <string>
#include <map>
#include <ostream>

class HTTPRequest
{
public:
    HTTPRequest();
    ~HTTPRequest();
    
    void initParameter();
    
    std::string toRequestMessage();
    void addRequestHeader(std::string key,std::string value);
public:
    std::string method;
    std::string path;
    std::string version;
    
    std::map<std::string,std::string> *header;
    
    std::basic_string<unsigned char> requestBody;
};

std::ostream & operator << (std::ostream &os,HTTPRequest *res);

std::ostream & operator << (std::ostream &os,HTTPRequest& res);

#endif
