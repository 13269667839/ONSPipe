#include "HTTPClient.hpp"

HTTPClient::HTTPClient(std::string _url,HTTPMethod _method)
{
    url = nullptr;
    httpRequest = nullptr;
    
    if (!_url.empty())
    {
        method = _method;
        url = new URL(_url);
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

HTTPResponse * HTTPClient::sendRequest()
{
    setHttpRequest();
    if (!httpRequest)
    {
        Utility::throwError("http request is not set");
    }
    
    auto socket = Socket(url->host, url->portNumber);
    if (!socket.connect())
    {
        Utility::throwError("can not connect to server");
    }
    
    setSocketConfig(socket);
    socket.sendAll(httpRequest->toRequestMessage());
    
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
        Utility::throwError("url is null");
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
        Utility::throwError("url's host is null");
    }
    httpRequest->addRequestHeader({"Host",url->host});
}
