#include "HTTPResponse.hpp"
#include "Utility.hpp"

HTTPResponse::HTTPResponse()
{
    setDefaultParameters();
}

void HTTPResponse::setDefaultParameters()
{
    httpVersion = std::string();
    statusCode = 0;
    reason = std::string();
    header = nullptr;
    responseBody = std::string();
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

void HTTPResponse::parseResponseLine(std::string line)
{
    auto arr = Utility::split(line, " ");
    if (arr.size() < 2)
    {
        Utility::throwError("invalid response line");
    }
    
    httpVersion = arr[0];
    statusCode = atoi(arr[1].c_str());
    
    for (auto ite = begin(arr) + 2;ite != end(arr);++ite)
    {
        reason += *ite;
    }
}

void HTTPResponse::parseResponseHead(std::string head)
{
    auto arr = Utility::split(head, "\r\n");
    if (!arr.empty())
    {
        header = new std::map<std::string,std::string>();
    }
    for (auto obj : arr)
    {
        auto pair = Utility::split(obj, ": ");
        if (pair.size() == 2)
        {
            auto key = pair[0];
            auto val = pair[1];
            
            if (!key.empty() && !val.empty())
            {
                header->insert({key,val});
            }
        }
    }
}

void HTTPResponse::parseResponseBody(std::string body)
{
    responseBody = body;
}

std::ostream & operator << (std::ostream &os,HTTPResponse *res)
{
    if (res)
    {
        os<<*res;
    }
    return os;
}

std::ostream & operator << (std::ostream &os,HTTPResponse res)
{
    os<<res.httpVersion<<' '<<res.statusCode<<' '<<res.reason<<std::endl;
    if (res.header)
    {
        for (auto pair : *res.header)
        {
            os<<pair.first<<" : "<<pair.second<<std::endl;
        }
    }
    os<<res.responseBody<<std::endl;
    return os;
}
