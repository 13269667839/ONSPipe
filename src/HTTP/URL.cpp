#include "URL.hpp"
#include "../Utility/Util.hpp"
#include <cstdlib>

URL::URL(std::string str)
{
    setInitialParameter();
    
    if (!str.empty())
    {
        originalURL = str;
        
        parseURLStr(str);
    }
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
        auto pair = Util::split(query, std::string("&"));
        for (auto str : pair)
        {
            auto kv = Util::split(str, std::string("="));
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
                    Util::throwError("invalid url");
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
                        auto arr = Util::split(host, std::string(":"));
                        if (arr.size() == 2)
                        {
                            host = arr[0];
                            portNumber = atoi(arr[1].c_str());
                        }
                        else
                        {
                            Util::throwError("invalid url");
                        }
                    }
                    else
                    {
                        if (scheme == "http" || scheme == "https")
                        {
                            portNumber = 80;
                        }
                    }
                    
                    curIdx = idx + 1;
                    
                    state = URLParseState::Path;
                }
                else
                {
                    Util::throwError("invalid url");
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
