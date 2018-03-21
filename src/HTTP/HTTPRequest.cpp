#include "HTTPRequest.hpp"
#include "../Utility/Util.hpp"

HTTPRequest::HTTPRequest()
{
    method = std::string();
    path = std::string();
    version = std::string();

    header = nullptr;

    body = std::basic_string<Util::byte>();
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
    method.clear();
    path.clear();
    version.clear();

    if (header)
    {
        header->clear();
    }

    body.clear();
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

void HTTPRequest::setRequestLine(std::string _method,std::string _path,std::string _version)
{
    method = _method;
    path = _path;
    version = _version;
}

std::string HTTPRequest::toRequestMessage()
{
    auto requestLine = method + " " + path + " " + version;

    auto headerStr = std::string();
    if (header && !header->empty())
    {
        for (auto pair : *header)
        {
            headerStr += pair.first + ": " + pair.second + "\r\n";
        }
    }

    return requestLine + "\r\n" + headerStr + "\r\n" + std::string(body.begin(), body.end());
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
