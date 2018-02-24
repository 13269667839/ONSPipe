#include "HTTPRequest.hpp"
#include "../Utility/Util.hpp"

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
    
    requestBody = std::basic_string<Util::byte>();
}

void HTTPRequest::addRequestHeader(std::string key,std::string value)
{
    if (!key.empty() && !value.empty())
    {
        if (!header)
        {
            header = new std::map<std::string,std::string>();
        }
        header->insert({key,value});
    }
}

std::string HTTPRequest::toRequestMessage()
{
    if (method.empty() || version.empty() || path.empty())
    {
        throwError("invalid http request line format");
    }
    auto requestLine = method + " " + path + " " + version;
    
    
    if (!header || header->empty())
    {
        throwError("invalid http request header format");
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
    
    return requestLine + "\r\n" + Util::join(arr, std::string("\r\n")) + "\r\n\r\n" + std::string(requestBody.begin(),requestBody.end());
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
