#include "Strings.hpp"
#include <clocale>
#include <map>
#include <cstring>

bool Strings::isPrefix(std::string str,std::string prefix)
{
    if (str.empty() || str.size() < prefix.size())
    {
        return false;
    }

    if (prefix.empty())
    {
        return true;
    }

    return strncmp(str.c_str(),prefix.c_str(),prefix.size()) == 0;
}

std::wstring Strings::s2ws(const std::string &s)
{
    auto curLocale = setlocale(LC_ALL, "");
    
    auto _Source = s.c_str();
    auto _Dsize = mbstowcs(nullptr, _Source, 0) + 1;
    auto _Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    auto result = std::wstring(_Dest);
    delete []_Dest;
    
    setlocale(LC_ALL, curLocale);
    return result;
}

unsigned long Strings::u_strlen(const char *utf8_str)
{
    unsigned long len = 0;
    
    if (!utf8_str)
    {
        return len;
    }
    
    auto size = strlen(utf8_str);
    if (size == 0)
    {
        return len;
    }
    
    for (decltype(size) i = 0;i < size;++i)
    {
        unsigned long tmp = utf8_str[i];
        unsigned int bits[8] = {};
        
        auto idx = 7;
        while (tmp != 0 && idx >= 0)
        {
            bits[idx] = tmp % 2;
            idx--;
            tmp /= 2;
        }
        
        if (bits[0] == 1)
        {
            auto count_one = 1;
            while (bits[count_one] == 1)
            {
                count_one++;
            }
            
            i += (count_one - 1);
        }
        
        len++;
    }
    
    return len;
}

std::vector<std::string> Strings::split(std::string src, std::vector<std::string> tokens)
{
    std::vector<std::string> arr;
    if (!src.empty() && !tokens.empty())
    {
        if (tokens.size() == 1)
        {
            arr = Strings::split(src, *begin(tokens));
        }
        else
        {
            auto capitalMap = std::map<char, std::vector<std::string>>();
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
                long lastIndex = -1;
                for (std::string::size_type i = 0; i < src.size(); ++i)
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
                                if (item == src.substr(i, item.size()))
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

                            auto subStr = src.substr(lastIndex, i - lastIndex);
                            if (!subStr.empty())
                            {
                                arr.push_back(subStr);
                            }

                            i += tok.size() - 1;
                            lastIndex = i + 1;
                        }
                    }
                }

                long size = src.size();
                if (lastIndex != -1 && lastIndex < size)
                {
                    auto subStr = src.substr(lastIndex);
                    if (!subStr.empty())
                    {
                        arr.push_back(subStr);
                    }
                }
            }
        }
    }
    return arr;
}