#ifndef Utility_hpp
#define Utility_hpp

#include <vector>
#include <string>
#include <map>

namespace Utility
{
#pragma mark -- STL extension
    std::vector<std::string> split(std::string src,std::string token);
    
    std::vector<std::string> split(std::string src,std::vector<std::string> tokens);
    
    std::string toLowerStr(std::string src);
    
    void throwError(std::string msg);
    
    std::string join(std::vector<std::string> srcArr,std::string token);
    
#pragma mark -- HTTP request and response state
    enum class HTTPMessageParseState : int
    {
        Init = 0,
        Line,
        Header,
        Body
    };
}

#endif
