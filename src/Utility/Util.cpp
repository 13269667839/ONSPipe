#include "Util.hpp"
#include <algorithm>
#include <cctype>
#include <exception>

std::vector<std::string> Util::split(std::string src,std::string token)
{
    std::vector<std::string> arr;
    if (!src.empty())
    {
        if (!token.empty())
        {
            auto tokenIndex = std::string::npos;
            for (std::string::size_type i = 0;i < src.size();++i)
            {
                tokenIndex = src.find(token,i);
                if (tokenIndex == std::string::npos)
                {
                    if (i < src.size())
                    {
                        auto subStr = src.substr(i);
                        arr.push_back(subStr);
                    }
                    break;
                }
                else if (tokenIndex > src.size())
                {
                    break;
                }
                else
                {
                    auto subStr = src.substr(i,tokenIndex - i);
                    arr.push_back(subStr);
                    
                    i = tokenIndex - 1 + token.size();
                }
            }
        }
        else
        {
            arr = std::vector<std::string>(src.size());
            for (int i = 0;i < arr.size();++i)
            {
                arr[i] = std::string({src[i]});
            }
        }
    }
    return arr;
}

std::vector<std::string> Util::split(std::string src,std::vector<std::string> tokens)
{
    std::vector<std::string> arr;
    if (!src.empty() && !tokens.empty())
    {
        if (tokens.size() == 1)
        {
            arr = Util::split(src, *begin(tokens));
        }
        else
        {
            auto capitalMap = std::map<char,std::vector<std::string>>();
            for (auto token : tokens)
            {
                char ch = *begin(token);
                if (ch != '\0')
                {
                    auto ite = capitalMap.find(ch);
                    if (ite == end(capitalMap))
                    {
                        capitalMap[ch] = {token};
                    }
                    else
                    {
                        ite->second.push_back(token);
                    }
                }
            }
            
            if (!capitalMap.empty())
            {
                std::string::size_type lastIndex = -1;
                for (std::string::size_type i = 0;i < src.size();++i)
                {
                    char ch = src[i];
                    
                    auto chain = capitalMap[ch];
                    if (!chain.empty())
                    {
                        std::string tok;
                        for (auto item : chain)
                        {
                            if (i + item.size() - 1 < src.size())
                            {
                                if (item == src.substr(i,item.size()))
                                {
                                    tok = item;
                                    break;
                                }
                            }
                        }
                        
                        if (!tok.empty())
                        {
                            if (lastIndex == -1)
                            {
                                lastIndex = 0;
                            }
                            
                            auto subStr = src.substr(lastIndex,i - lastIndex);
                            arr.push_back(subStr);
                            
                            i += tok.size() - 1;
                            lastIndex = i + 1;
                        }
                    }
                }
                
                if (lastIndex != -1 && lastIndex < src.size())
                {
                    auto subStr = src.substr(lastIndex);
                    arr.push_back(subStr);
                }
            }
        }
    }
    return arr;
}

std::string Util::toLowerStr(std::string src)
{
    auto res = std::string();
    if (!src.empty())
    {
        auto chars = std::vector<char>(src.size());
        std::transform(begin(src), end(src), begin(chars), [](int ch)
                       {
                           if (isupper(ch))
                           {
                               return tolower(ch);
                           }
                           return ch;
                       });
        
        if (!chars.empty())
        {
            res = std::string(begin(chars),end(chars));
        }
    }
    return res;
}

void Util::throwError(std::string msg)
{
    throw std::logic_error(msg);
}

std::string Util::join(std::vector<std::string> srcArr,std::string token)
{
    auto res = std::string();
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