#ifndef HTTPParser_hpp
#define HTTPParser_hpp

#include "Util.hpp"
#include <deque>
#include "../HTTP/HTTPResponse.hpp"
#include "../HTTP/HTTPRequest.hpp"

#pragma mark -- HTTPRecvMsgParser
class HTTPRecvMsgParser
{
public:
    HTTPRecvMsgParser();
    ~HTTPRecvMsgParser();
    
    bool is_parse_msg();
    HTTPResponse * msg2res();
private:
    bool parse_line();
    bool parse_header();
    bool parse_body();
public:
    std::deque<Util::byte> *cache;
    
    std::string version;
    std::string status_code;
    std::string reason;
    
    std::map<std::string,std::string> header;
private:
    HTTPMessageParseState state;
    long content_length;
    bool isChunk;
};

#pragma mark -- HTTPReqMsgParser
class HTTPReqMsgParser
{
public:
    HTTPReqMsgParser();
    ~HTTPReqMsgParser();
    
    bool is_parse_msg();
    
    void msg2req(HTTPRequest &req);
    void initParams();
public:
    std::deque<Util::byte> *cache;
    
    std::string method;
    std::string path;
    std::string version;
    
    std::map<std::string,std::string> header;
private:
    HTTPMessageParseState state;
    long content_length;
private:
    bool parse_line();
    bool parse_header();
    bool parse_body();
};

#endif
