#ifndef HTTPRequest_hpp
#define HTTPRequest_hpp

#include <string>
#include <map>
#include <ostream>
#include "../Utility/Util.hpp"

class HTTPRequest
{
public:
    HTTPRequest();
    ~HTTPRequest();
    
    void initParameter();
    
    std::string toRequestMessage();
    void addRequestHeader(std::pair<std::string,std::string> pair);
public:
    std::string method;
    std::string path;
    std::string version;
    
    std::map<std::string,std::string> *header;
    
    std::string requestBody;
};

std::ostream & operator << (std::ostream &os,HTTPRequest *res);

std::ostream & operator << (std::ostream &os,HTTPRequest& res);

#endif
