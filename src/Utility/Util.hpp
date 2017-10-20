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
    static std::vector<std::string> split(std::string src,std::string token);
    
    static std::vector<std::string> split(std::string src,std::vector<std::string> tokens);
    
    static std::string toLowerStr(std::string src);
    
    static void throwError(std::string msg);
    
    static std::string join(std::vector<std::string> srcArr,std::string token);
    
    static std::vector<char> readFileSync(std::string filePath);

    static std::string currentWorkDirectory();

    static bool isDirectory(std::string path);

    static std::vector<std::string> filesInTheCurrentDirectory(std::string filePath);
};

#endif
