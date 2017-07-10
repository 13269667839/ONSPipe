#ifndef Util_hpp
#define Util_hpp

#include <vector>
#include <string>
#include <map>

#pragma mark -- HTTP request and response state
enum class HTTPMessageParseState : int
{
    Init = 0,
    Line,
    Header,
    Body
};

class Util
{
public:
#pragma mark -- STL extension
    static std::vector<std::string> split(std::string src,std::string token);
    
    static std::vector<std::string> split(std::string src,std::vector<std::string> tokens);
    
    static std::string toLowerStr(std::string src);
    
    static void throwError(std::string msg);
    
    static std::string join(std::vector<std::string> srcArr,std::string token);
};

#endif
