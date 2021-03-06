#ifndef HTTPReqMsgParser_hpp
#define HTTPReqMsgParser_hpp

#include <deque>
#include "../HTTP/HTTPRequest.hpp"
#include "../Utility/UtilConstant.hpp"

class HTTPReqMsgParser
{
public:
    HTTPReqMsgParser();
    ~HTTPReqMsgParser();
    
    bool is_parse_msg();
    
    void msg2req(HTTPRequest &req);
    void initParams();

    void addToCache(std::vector<Util::byte> &bytes);
public:
    std::string method;
    std::string path;
    std::string version;
    
    std::map<std::string,std::string> header;
private:
    std::deque<Util::byte> *cache;
    HTTPMessageParseState state;
    long content_length;
private:
    bool parse_line();
    bool parse_header();
    bool parse_body();
};

#endif
