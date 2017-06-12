#ifndef HTTPRequest_hpp
#define HTTPRequest_hpp

#include <string>
#include <map>
#include <ostream>

enum class HTTPReqMsgParseState : int
{
    Init = 0,
    Line,
    Header,
    Body
};

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
    std::string HTTPVersion;
    
    // === head ===
    std::map<std::string,std::string> *header;
    
    // === body ===
    std::string query;
private:
    std::string requestLine();
    std::string requestHeader();
    
    HTTPReqMsgParseState parseState;
    std::string recvHTTPReqMsgBuf;
    long content_length;
};

#endif
