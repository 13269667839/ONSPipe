#include "HTTP.hpp"
#include "../Socket/Socket.hpp"
#include "../Utility/Utility.hpp"

HTTP::HTTP(std::string _url,HTTPMethod _method)
{
    url = nullptr;
    header = nullptr;
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
    HTTPResponse *res = nullptr;
    auto rawStr = rawHTTPStr();
    if (!rawStr.empty())
    {
        auto socket = Socket(url->host, url->portNumber);
        if (socket.connect())
        {
            int recvBufSize = 1024 * 5;
            socket.recvBuffSize = recvBufSize;
            socket.setSocketOpt(SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize));
            
            socket.sendAll(rawStr);
            
            auto strBuf = std::string();
            int status = 0;
            while (1)
            {
                auto recvbuf = socket.receive();
                if (!recvbuf)
                {
                    if (!strBuf.empty() && status == 2 && res)
                    {
                        res->parseResponseBody(strBuf);
                    }
                    break;
                }
                else
                {
                    auto tmpBuf = static_cast<char *>(recvbuf);
                    strBuf += tmpBuf;
                    delete tmpBuf;
                    
                    if (!strBuf.empty())
                    {
                        if (status == 0)
                        {
                            auto idx = strBuf.find("\r\n");
                            if (idx != std::string::npos)
                            {
                                status = 1;
                                auto line = strBuf.substr(0,idx);
                                if (!res)
                                {
                                    res = new HTTPResponse();
                                }
                                res->parseResponseLine(line);
                                strBuf = strBuf.substr(idx + 2);
                            }
                        }
                        else if (status == 1)
                        {
                            auto idx = strBuf.find("\r\n\r\n");
                            if (idx != std::string::npos)
                            {
                                status = 2;
                                auto head = strBuf.substr(0,idx);
                                res->parseResponseHead(head);
                                strBuf = strBuf.substr(idx + 4);
                            }
                        }
                    }
                }
            }
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
    return "";
}

void HTTP::addRequestHeader(std::string key,std::string val)
{
    if (header && !key.empty() && !val.empty())
    {
        header->insert({key,val});
    }
}
