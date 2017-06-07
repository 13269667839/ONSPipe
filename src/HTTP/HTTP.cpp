#include "HTTP.hpp"
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
    auto strBuf = std::string();
    
    //recv msg parser machine's status
    int status = 0;
    
    //content-length
    size_t size = 0;
    size_t content_size = -1;
    
    //transfer-encoding = chunked
    bool isChunked = false;
    
    while (1)
    {
        auto recvbuf = socket.receive();
        if (!recvbuf)
        {
            if (status == 2 && res)
            {
                res->responseBody = strBuf;
            }
            break;
        }
        else
        {
            auto tmpBuf = static_cast<char *>(recvbuf);
            size += strlen(tmpBuf);
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
                        res = parseResponseLine(strBuf, idx);
                        if (!res)
                        {
                            break;
                        }
                        
                        auto headEndIndex = strBuf.find("\r\n\r\n");
                        if (res->statusCode != 202 || (headEndIndex != std::string::npos && headEndIndex + 4 == strBuf.size()))
                        {
                            goto ParseResponseMessageHead;
                        }
                    }
                }
                else if (status == 1)
                {
                ParseResponseMessageHead:
                    auto idx = strBuf.find("\r\n\r\n");
                    if (idx != std::string::npos)
                    {
                        status = 2;
                        
                        auto resHead = parseResponseHead(strBuf, idx);
                        if (resHead)
                        {
                            res->header = resHead;
                            
                            auto ite = res->header->find("content-length");
                            if (ite != end(*res->header))
                            {
                                auto value = ite->second;
                                if (!value.empty())
                                {
                                    content_size = atoi(value.c_str());
                                }
                            }
                            else
                            {
                                auto chunkIte = res->header->find("transfer-encoding");
                                if (chunkIte != end(*res->header))
                                {
                                    auto value = chunkIte->second;
                                    if (Utility::toLowerStr(value) == "chunked")
                                    {
                                        isChunked = true;
                                    }
                                }
                            }
                        }
                        
                    }
                }
            }
        }
        
        if (res && status == 2)//close connect at clinet
        {
            bool canCloseConnect = false;
            if (content_size != -1)//Content-Length: xxx
            {
                if (size >= content_size)
                {
                    canCloseConnect = true;
                }
            }
            else if (isChunked)//transfer-encoding: chunked
            {
                if (strBuf.rfind("\r\n0\r\n\r\n") != std::string::npos)
                {
                    canCloseConnect = true;
                }
            }
            
            if (canCloseConnect)
            {
                res->responseBody = strBuf;
                break;
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

void HTTP::setSocketConfig(Socket &socket)
{
    int recvBufSize = 1024 * 10;
    socket.recvBuffSize = recvBufSize;
    socket.setSocketOpt(SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize));
}

HTTPResponse * HTTP::parseResponseLine(std::string &buf,std::string::size_type idx)
{
    auto res = new HTTPResponse();
    if (idx < buf.size())
    {
        auto line = buf.substr(0,idx);
        
        if (!line.empty())
        {
            auto arr = Utility::split(line, " ");
            if (arr.size() < 2)
            {
                Utility::throwError("invalid response line");
            }
            
            res->httpVersion = arr[0];
            res->statusCode = atoi(arr[1].c_str());
            
            for (auto ite = begin(arr) + 2;ite != end(arr);++ite)
            {
                res->reason += *ite;
            }
        }
        
        if (idx + 2 < buf.size())
        {
            buf = buf.substr(idx + 2);
        }
    }
    return res;
}

std::map<std::string,std::string> * HTTP::parseResponseHead(std::string &buf,std::string::size_type idx)
{
    std::map<std::string,std::string> *res = nullptr;
    if (!buf.empty() && idx < buf.size())
    {
        auto head = buf.substr(0,idx);
        
        if (!head.empty())
        {
            auto arr = Utility::split(head, "\r\n");
            
            if (!arr.empty())
            {
                res = new std::map<std::string,std::string>();
                
                for (auto obj : arr)
                {
                    auto pair = Utility::split(obj, ": ");
                    if (pair.size() == 2)
                    {
                        auto key = pair[0];
                        auto val = pair[1];
                        
                        if (!key.empty() && !val.empty())
                        {
                            res->insert({Utility::toLowerStr(key),val});
                        }
                    }
                }
            }
        }
        
        auto endIndex = idx + 4;
        if (endIndex <= buf.size())
        {
            buf = buf.substr(endIndex);
        }
    }
    return res;
}
