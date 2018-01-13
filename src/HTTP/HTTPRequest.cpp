#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
{
    header = nullptr;
    
    initParameter();
}

HTTPRequest::~HTTPRequest()
{
    if (header)
    {
        header->clear();
        delete header;
        header = nullptr;
    }
}

void HTTPRequest::initParameter()
{
    method = "GET";
    path = "/";
    version = "HTTP/1.1";
    
    if (header)
    {
        header->clear();
    }
    else
    {
        header = nullptr;
    }
    
    requestBody = std::string();
}

void HTTPRequest::addRequestHeader(std::pair<std::string,std::string> pair)
{
    if (!header)
    {
        header = new std::map<std::string,std::string>();
    }
    
    if (!pair.first.empty() && !pair.second.empty())
    {
        header->insert(pair);
    }
}

std::string HTTPRequest::toRequestMessage()
{
    if (method.empty() || version.empty() || path.empty())
    {
        Util::throwError("invalid http request line format");
    }
    auto requestLine = method + " " + path + " " + version;
    
    
    if (!header || header->empty())
    {
        Util::throwError("invalid http request header format");
        return "";
    }
    auto arr = std::vector<std::string>();
    for (auto pair : *header)
    {
        if (!pair.first.empty() && !pair.second.empty())
        {
            arr.push_back(pair.first + ": " + pair.second);
        }
    }
    
    return requestLine + "\r\n" + Util::join(arr, std::string("\r\n")) + "\r\n\r\n" + requestBody;
}

std::ostream & operator << (std::ostream &os,HTTPRequest *res)
{
    os<<*res;
    return os;
}

std::ostream & operator << (std::ostream &os,HTTPRequest& res)
{
    os<<res.toRequestMessage();
    return os;
}
