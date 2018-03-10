#include "URL.hpp"
#include "../Utility/Util.hpp"
#include <cstdlib>

URL::URL(std::string str)
{
    setInitialParameter();
    
    if (!str.empty())
    {
        auto url = urlEncode(str);
        originalURL += url;
        parseURLStr(url);
    }
}

std::string URL::urlEncode(std::string &str)
{
    auto encodeURL = std::string();

    for (std::string::size_type i = 0;i < str.size();++i)
    {
        auto ch = str[i];
        if (isalnum(ch))
        {
            encodeURL += ch;
        }
        else if (isspace(ch))
        {
            encodeURL += "+";
        }
        else if (isascii(ch))
        {
            encodeURL += ch;
        }
        else 
        {
            Util::byte tmp = ch;
            int bits[8];
     
            auto idx = 7;
            while (tmp != 0 && idx >= 0)
            {
                bits[idx] = tmp % 2;
                idx--;
                tmp /= 2;
            }

            int len = 1;
            if (bits[0] == 1)
            {
                while (bits[len] == 1)
                {
                    len++;
                }
            }

            auto k = 0;
            while (k < len)
            {
                char cache[12];
                sprintf(cache,"%%%02X",(Util::byte)str[i + k]);
                encodeURL += cache;
                ++k;
            }

            i += (len - 1);
        }
    }

    return encodeURL;
}

void URL::setInitialParameter()
{
    originalURL = std::string();
    scheme = std::string();
    host = std::string();
    portNumber = -1;
    path = std::string("/");
    query = std::string();
}

std::map<std::string,std::string> URL::queryDic()
{
    auto dic = std::map<std::string,std::string>();

    if (!query.empty())
    {
        auto pair = Strings::split(query, std::string("&"));
        for (auto str : pair)
        {
            auto kv = Strings::split(str, std::string("="));
            if (kv.size() == 2)
            {
                dic[kv[0]] = kv[1];
            }
        }
    }

    return dic;
}

enum class URLParseState : int
{
    Initialize = 0,
    Scheme,
    Host,
    Path,
    Query
};

void URL::parseURLStr(std::string urlStr)
{
    URLParseState state = URLParseState::Initialize;
    std::string::size_type curIdx = 0;
    
    while (1)
    {
        switch (state)
        {
            case URLParseState::Initialize:
            {
                if (urlStr.find("://") != std::string::npos)
                {
                    state = URLParseState::Scheme;
                }
                else
                {
                    throwError("invalid url");
                }
            }
            break;
            case URLParseState::Scheme:
            {
                auto idx = urlStr.find("://");
                scheme = urlStr.substr(0,idx);
                curIdx = idx + 3;
                state = URLParseState::Host;
            }
            break;
            case URLParseState::Host:
            {
                auto idx = urlStr.find("/",curIdx);
                
                if (idx != std::string::npos)
                {
                    host = urlStr.substr(curIdx,idx - curIdx);
                    
                    auto colonIdx = host.find(":");
                    if (colonIdx != std::string::npos)
                    {
                        auto arr = Strings::split(host, std::string(":"));
                        if (arr.size() == 2)
                        {
                            host = arr[0];
                            portNumber = atoi(arr[1].c_str());
                        }
                        else
                        {
                            throwError("invalid url");
                        }
                    }
                    else
                    {
                        if (scheme == "http")
                        {
                            portNumber = 80;
                        }
                        else if (scheme == "https")
                        {
                            portNumber = 443;
                        }
                    }
                    
                    curIdx = idx + 1;
                    
                    state = URLParseState::Path;
                }
                else
                {
                    throwError("invalid url");
                }
            }
            break;
            case URLParseState::Path:
            {
                auto idx = urlStr.find("?",curIdx);
                if (idx != std::string::npos)
                {
                    path += urlStr.substr(curIdx,idx - curIdx);
                    curIdx = idx + 1;
                }
                else
                {
                    path += urlStr.substr(curIdx);
                    curIdx = urlStr.size();
                }
                state = URLParseState::Query;
            }
            break;
            case URLParseState::Query:
            {
                query = urlStr.substr(curIdx);
                curIdx = urlStr.size();
            }
            break;
            default:
                break;
        }
        
        if (curIdx == urlStr.size())
        {
            break;
        }
    }
}

std::ostream & operator << (std::ostream &os,URL url)
{
    os<<"original url : "<<url.originalURL<<std::endl;
    os<<"scheme : "<<url.scheme<<std::endl;
    os<<"host : "<<url.host<<std::endl;
    os<<"port : "<<url.portNumber<<std::endl;
    os<<"path : "<<url.path<<std::endl;
    os<<"query : "<<url.query<<std::endl;
    return os;
} 
