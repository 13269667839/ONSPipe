#include "URL.hpp"
#include "Utility.hpp"
#include <cstdlib>

URL::URL(std::string str)
{
    setInitialParameter();
    
    if (!str.empty())
    {
        originalURL = str;
        
        auto schemeStep = setScheme(str);
        auto hostStep = setHost(schemeStep);
        auto pathStep = setPath(hostStep);
        setQuery(pathStep);
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

std::string URL::setScheme(std::string str)
{
    auto res = std::string();
    auto idx = str.find("://");
    if (idx != std::string::npos)
    {
        scheme = str.substr(0,idx);
        scheme = Utility::toLowerStr(scheme);
        if (idx + 3 < str.size())
        {
            res = str.substr(idx + 3);
        }
    }
    else
    {
        Utility::throwError("invalid url");
    }
    return res;
}

std::string URL::setHost(std::string str)
{
    auto res = std::string();
    if (!str.empty())
    {
        auto arr = Utility::split(str, "/");
        if (!arr.empty())
        {
            host = arr[0];
            
            auto idx = host.find(":");
            if (idx != std::string::npos)
            {
                portNumber = atoi(host.substr(idx + 1).c_str());
                host = host.substr(0,idx);
            }
            else
            {
                if (scheme == "http" || scheme == "https")
                {
                    portNumber = 80;
                }
            }
            
            if (arr.size() > 1)
            {
                arr.erase(begin(arr));
                res = Utility::join(arr, "/");
            }
        }
    }
    return res;
}

std::string URL::setPath(std::string str)
{
    auto res = std::string();
    if (!str.empty())
    {
        auto idx = str.find("?");
        if (idx != std::string::npos)
        {
            path += str.substr(0,idx);
            res = str.substr(idx+1);
        }
        else
        {
            path += str;
            res = "";
        }
    }
    return res;
}

void URL::setQuery(std::string str)
{
    if (!str.empty())
    {
        query = str;
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
