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
    template <typename strType>
    static std::vector<strType> split(strType src, strType token)
    {
        std::vector<strType> arr;
        if (!src.empty())
        {
            if (!token.empty())
            {
                auto tokenIndex = strType::npos;
                for (unsigned long i = 0; i < src.size(); ++i)
                {
                    tokenIndex = src.find(token, i);
                    if (tokenIndex == strType::npos)
                    {
                        if (i < src.size())
                        {
                            arr.push_back(src.substr(i));
                        }
                        break;
                    }
                    else if (tokenIndex > src.size())
                    {
                        break;
                    }
                    else
                    {
                        auto subStr = src.substr(i, tokenIndex - i);
                        arr.push_back(subStr);
                        
                        i = tokenIndex - 1 + token.size();
                    }
                }
            }
            else
            {
                arr = std::vector<strType>(src.size());
                for (int i = 0; i < arr.size(); ++i)
                {
                    arr[i] = strType({src[i]});
                }
            }
        }
        return arr;
    }
    
    static std::vector<std::string> split(std::string src,std::vector<std::string> tokens);
    
    template <typename strType>
    static strType toLowerStr(strType src)
    {
        for (auto i = 0;i < src.length();++i) 
        {
            if (isupper(src[i]))
            {
                src[i] = tolower(src[i]);
            }
        }
        return src;
    }
    
    static void throwError(std::string msg);
    
    template <typename strType>
    static strType join(std::vector<strType> srcArr, strType token)
    {
        auto res = strType();
        if (!srcArr.empty())
        {
            const bool empty = token.empty();
            for (auto src : srcArr)
            {
                auto itemStr = src;
                if (!empty && src != *(end(srcArr) - 1))
                {   
                    itemStr += token;
                }
                res += itemStr;
            }
        }
        return res;
    }
    
    static std::vector<char> readFileSync(std::string filePath);

    static std::string currentWorkDirectory();

    static bool isDirectory(std::string path);

    static std::vector<std::string> filesInTheCurrentDirectory(std::string filePath);

    static std::map<std::string,std::string> Argv2Map(const char * argv[],int len,const std::map<std::string,int> rule);
    
    static std::wstring s2ws(const std::string &s);
    
    static unsigned long u_strlen(const char *utf8_str);
};

#endif
