#ifndef Strings_hpp
#define Strings_hpp

#include <string>
#include <vector>

class Strings 
{
public:
    static bool isPrefix(std::string str,std::string prefix);

    static std::wstring s2ws(const std::string &s);
    
    static unsigned long u_strlen(const char *utf8_str);

    static std::vector<std::string> split(std::string src,std::vector<std::string> tokens);

    template <typename charType>
    static void trimLeft(std::basic_string<charType> &src, const charType ch)
    {
        auto ite = src.begin();
        while (ite != src.end())
        {
            if (*ite != ch)
            {
                break;
            }
            ite = src.erase(ite);
        }
    }

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

    template <typename charType>
    static void trim(std::basic_string<charType> &src, const charType ch)
    {
        trimLeft(src, ch);
        trimRight(src, ch);
    }

    template <typename strType>
    static strType toLowerStr(strType src)
    {
        for (decltype(src.length()) i = 0; i < src.length(); ++i)
        {
            if (isupper(src[i]))
            {
                src[i] = tolower(src[i]);
            }
        }
        return src;
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

    template <typename strType>
    static strType join(std::vector<strType> srcArr, strType token)
    {
        auto res = strType();

        if (srcArr.empty())
        {
            return res;
        }

        auto end = srcArr.end() - 1;
        for (auto ite = srcArr.begin(); ite != end; ++ite)
        {
            res += (*ite + token);
        }

        res += *end;

        return res;
    }
};

#endif