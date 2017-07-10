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
    
    std::string toRequestMessage();
    void addRequestHeader(std::pair<std::string,std::string> pair);
    
    bool parseRequestMessage(std::string reqMsg);
    
    friend std::ostream & operator << (std::ostream &os,HTTPRequest *res);
    friend std::ostream & operator << (std::ostream &os,HTTPRequest& res);
public:
    // === line ===
    std::string HTTPMethod;
    std::string path;
    std::string httpVersion;
    
    // === head ===
    std::map<std::string,std::string> *header;
    
    // === body ===
    std::string query;
private:
    std::string requestLine();
    std::string requestHeader();
    
    HTTPMessageParseState parseState;
    std::string recvHTTPReqMsgBuf;
    long content_length;
};

#endif
