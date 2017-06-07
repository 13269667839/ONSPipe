#include "HTTPResponse.hpp"
#include "Utility.hpp"

HTTPResponse::HTTPResponse()
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
    os<<res.toResponseMessage();
    return os;
}

std::string HTTPResponse::toResponseMessage()
{
    auto line = httpVersion + " " + std::to_string(statusCode) + " " + reason;
    
    auto head = std::string();
    if (header && !header->empty())
    {
        auto arr = std::vector<std::string>();
        for (auto pair : *header)
        {
            arr.push_back(pair.first + ": " + pair.second);
        }
        head = Utility::join(arr, "\r\n");
    }

    return line + "\r\n" + head + "\r\n\r\n" + responseBody + "\r\n";
}
