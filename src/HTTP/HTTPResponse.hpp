#ifndef Response_hpp
#define Response_hpp

#include <string>
#include <ostream>
#include <map>
#include "../Utility/Util.hpp"

class HTTPResponse
{
public:
    HTTPResponse();
    ~HTTPResponse();
    friend std::ostream & operator << (std::ostream &os,HTTPResponse *res);
    friend std::ostream & operator << (std::ostream &os,HTTPResponse &res);
    
    std::string toResponseMessage();
    
    bool parseHttpResponseMsg(std::string msg);
    void addResponseHead(std::pair<std::string,std::string> pair);
public:
    //=== line ===
    std::string httpVersion;
    int statusCode;
    std::string reason;
    
    //=== head ===
    std::map<std::string,std::string> *header;
    
    //=== body ===
    std::string responseBody;
    
private:
    HTTPMessageParseState state;
    std::string recvBuf;
    
    long content_length;
    long recvMsgLen;
    
    bool isChunk;
private:
    bool initState();
    bool lineState();
    bool headerState();
    bool bodyState();
    
    void removeChunkBodyMsg();
};

#endif
