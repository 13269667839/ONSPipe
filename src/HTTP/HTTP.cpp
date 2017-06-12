#include "HTTP.hpp"

HTTP::HTTP(std::string _url,HTTPMethod _method)
{
    url = nullptr;
    header = nullptr;
    requestQuery = "";
    if (!_url.empty())
    {
        method = _method;
        url = new URL(_url);
        setDefaultHeader();
    }
}

HTTP::~HTTP()
{
    if (url)
    {
        delete url;
        url = nullptr;
    }
    
    if (header)
    {
        header->clear();
        header = nullptr;
    }
}

HTTPResponse * HTTP::request()
{
    auto rawStr = rawHTTPStr();
    if (rawStr.empty())
    {
        Utility::throwError("raw request message is empty");
    }
    
    auto socket = Socket(url->host, url->portNumber);
    if (!socket.connect())
    {
        Utility::throwError("can not connect to server");
    }

    setSocketConfig(socket);
    socket.sendAll(rawStr);

    HTTPResponse *res = nullptr;
    while (1)
    {
        auto recvbuf = socket.receive();
        if (!recvbuf)
        {
            break;
        }
        else
        {
            auto strBuf = static_cast<char *>(recvbuf);
            
            if (!res)
            {
                res = new HTTPResponse();
            }
            
            if (res->parseHttpResponseMsg(strBuf))
            {
                delete strBuf;
                break;
            }
            
            delete strBuf;
        }
    }

    return res;
}

std::string HTTP::rawHTTPStr()
{
    return requestLine() + "\r\n" + requestHead() + "\r\n\r\n" + requestBody();
}

std::string HTTP::requestLine()
{
    auto components = std::vector<std::string>(3);
    components[0] = method == HTTPMethod::GET?"GET":"POST";
    if (url && !url->path.empty())
    {
        auto address = std::string(url->path);
        if (!url->query.empty())
        {
            address += "?" + url->query;
        }
        components[1] = address;
    }
    else
    {
        Utility::throwError("url is null");
    }
    components[2] = "HTTP/1.1";
    return Utility::join(components, " ");
}

void HTTP::setDefaultHeader()
{
    if (!url || url->host.empty())
    {
        Utility::throwError("url error");
    }
    
    header = new std::map<std::string,std::string>();
    header->insert({"Host",url->host});
}

std::string HTTP::requestHead()
{
    auto arr = std::vector<std::string>();
    for (auto pair : *header)
    {
        if (!pair.first.empty() && !pair.second.empty())
        {
            arr.push_back(pair.first + ": " + pair.second);
        }
    }
    return Utility::join(arr, "\r\n");
}

std::string HTTP::requestBody()
{
    return requestQuery;
}

void HTTP::addRequestHeader(std::string key,std::string val)
{
    if (header && !key.empty() && !val.empty())
    {
        header->insert({key,val});
    }
}

void HTTP::setSocketConfig(Socket &socket)
{
    int recvBufSize = 1024 * 10;
    socket.recvBuffSize = recvBufSize;
    socket.setSocketOpt(SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize));
}
