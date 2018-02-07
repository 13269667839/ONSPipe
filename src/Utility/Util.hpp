#ifndef Util_hpp
#define Util_hpp

#include <vector>
#include <string>
#include <map>
#include <exception>

#define throwError(msg) throw std::logic_error(msg)
    
enum class InputType : int
{
    File = 0,
    Text
};

class Util
{
public:
    using byte = unsigned char;
    
    template <typename charType>
    static void trimRight(std::basic_string<charType> &src,const charType ch)
    {
        auto rite = src.rbegin();
        while (rite != src.rend())
        {
            if (*rite != ch)
            {
                break;
            }
            rite = decltype(rite)(src.erase((++rite).base()));
        }
    }
    
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
                for (unsigned long i = 0; i < arr.size(); ++i)
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
    
    static char * base64_encoding(const char *buffer, int length, bool newLine);
    static byte * sha1_encode(byte *src,size_t len);
};

#endif
