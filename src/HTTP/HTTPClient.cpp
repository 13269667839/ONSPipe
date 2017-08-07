#include "HTTPClient.hpp"

HTTPClient::HTTPClient(std::string _url,HTTPMethod _method)
{
    url = nullptr;
    httpRequest = nullptr;
    
    if (!_url.empty())
    {
        method = _method;
        url = new URL(_url);
        setHttpRequest();
    }
}

HTTPClient::~HTTPClient()
{
    if (url)
    {
        delete url;
        url = nullptr;
    }
    
    if (httpRequest)
    {
        delete httpRequest;
        httpRequest = nullptr;
    }
}

void HTTPClient::setRequestHeader(std::string key,std::string value)
{
    if (key.empty() || value.empty() || !httpRequest)
    {
        return;
    }
    
    httpRequest->addRequestHeader({key,value});
}

HTTPResponse * HTTPClient::syncRequest()
{
    if (!httpRequest)
    {
        Util::throwError("http request is not set");
        return nullptr;
    }
    else if (!url || url->path.empty())
    {
        Util::throwError("url is null");
        return nullptr;
    }
    
    auto socket = Socket(url->host, url->portNumber);
    if (!socket.connect())
    {
        Util::throwError("can not connect to server");
    }
    
    setSocketConfig(socket);

    auto clientMsg = httpRequest->toRequestMessage();
    if (clientMsg.empty())
    {
        Util::throwError("http request message is null");
        return nullptr;
    }
    auto len = clientMsg.size();
    while (len > 0)
    {
        auto bytes = socket.send(clientMsg);
        if (bytes >= 0)
        {
            len -= bytes;
            if (bytes < clientMsg.size())
            {
                clientMsg = clientMsg.substr(bytes);
            }
        }
        else
        {
            Util::throwError("socket send error : " + std::string(gai_strerror(errno)));
            return nullptr;
        }
    }
    
    HTTPResponse *res = nullptr;
    while (1)
    {
        void *recvbuf = nullptr;
        long bytes = -2;
        
        std::tie(recvbuf,bytes) = socket.receive();
        
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

void HTTPClient::setSocketConfig(Socket &socket)
{
    int recvBufSize = 1024 * 10;
    socket.recvBuffSize = recvBufSize;
    socket.setSocketOpt(SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize));
}

void HTTPClient::setHttpRequest()
{
    if (!url || url->path.empty())
    {
        Util::throwError("url is null");
        return;
    }
    
    if (!httpRequest)
    {
        httpRequest = new HTTPRequest();
    }
    
    //=== line ===
    httpRequest->HTTPMethod = method == HTTPMethod::GET?"GET":"POST";
    
    httpRequest->path = url->path;
    if (!url->query.empty())
    {
        httpRequest->path += "?" + url->query;
    }
    
    httpRequest->httpVersion = "HTTP/1.1";
    
    //=== header ===
    if (url->host.empty())
    {
        Util::throwError("url's host is null");
    }
    setRequestHeader("Host", url->host);
}