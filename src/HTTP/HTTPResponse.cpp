#include "HTTPResponse.hpp"
#include "../Utility/Util.hpp"

HTTPResponse::HTTPResponse()
{
    header = nullptr;
    
    initParameter();
}

void HTTPResponse::initParameter()
{
    version = std::string();
    statusCode = 0;
    reason = std::string();
    if (header)
    {
        header->clear();
    }
    else
    {
        header = nullptr;
    }
    responseBody = std::basic_string<Util::byte>();
}

HTTPResponse::~HTTPResponse()
{
    if (header)
    {
        header->clear();
        delete header;
        header = nullptr;
    }
}

std::ostream & operator << (std::ostream &os,HTTPResponse *res)
{
    if (res)
    {
        os<<*res;
    }
    return os;
}

std::ostream & operator << (std::ostream &os,HTTPResponse &res)
{
    os<<res.toResponseMessage();
    return os;
}

std::string HTTPResponse::toResponseMessage()
{
    auto line = version + " " + std::to_string(statusCode) + " " + reason;
    
    auto head = std::string();
    if (header && !header->empty())
    {
        auto arr = std::vector<std::string>();
        for (auto pair : *header)
        {
            arr.push_back(pair.first + ": " + pair.second);
        }
        head = Util::join(arr, std::string("\r\n"));
    }

    return line + "\r\n" + head + "\r\n\r\n" + std::string(responseBody.begin(),responseBody.end()) + "\r\n";
}

void HTTPResponse::addResponseHead(std::string key,std::string value)
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
