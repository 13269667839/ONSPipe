#include "HTTPClient.hpp"
#include "../Utility/HTTPRecvMsgParser.hpp"

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
    if (httpRequest)
    {
        httpRequest->addRequestHeader(key,value);
    }
}

void HTTPClient::checkParams()
{
    if (!httpRequest)
    {
        throwError("http request is not set");
    }
    else if (!url || url->path.empty())
    {
        throwError("url is null");
    }
}

std::unique_ptr<HTTPResponse> HTTPClient::request()
{
    checkParams();
    
    auto socket = Socket(url->host, url->portNumber);

    auto https = url->scheme == "https";
    
    if (https)
    {
        socket.ssl_config(0);
    }

    if (!socket.connect())
    {
        throwError("can not connect to server");
    }
    
    setSocketConfig(socket);

    if (https) 
    {
        socket.ssl_set_fd(socket.socketfd);
        socket.ssl_connect();

#ifdef DEBUG
        socket.ssl_certification_info();
#endif
    }

    auto clientMsg = httpRequest->toRequestMessage();
    if (clientMsg.empty())
    {
        throwError("http request message is null");
        return nullptr;
    }
    
    socket.sendAll(const_cast<char *>(clientMsg.c_str()),clientMsg.size(),https);
    
    HTTPResponse *res = nullptr;
    auto parser = HTTPRecvMsgParser();
    parser.method = methodStr();
    while (1)
    {
        auto recvbuf = https ? (socket.ssl_read()) : (socket.receive());
        parser.addToCache(recvbuf);
        if (recvbuf.empty() || parser.is_parse_msg())
        {
            res = parser.msg2res();
            break;
        }
    }

    return std::unique_ptr<HTTPResponse>(res);
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
        throwError("url is null");
        return;
    }
    
    if (!httpRequest)
    {
        httpRequest = new HTTPRequest();
    }
    
    //=== line ===
    httpRequest->method = methodStr();
    
    httpRequest->path = url->path;
    if (!url->query.empty())
    {
        httpRequest->path += "?" + url->query;
    }
    
    httpRequest->version = "HTTP/1.1";
    
    //=== header ===
    if (url->host.empty())
    {
        throwError("url's host is null");
    }
    setRequestHeader("Host", url->host);
}

std::string HTTPClient::methodStr()
{
    auto res = std::string();
    switch (method)
    {
        case HTTPMethod::GET:
            res += "GET";
            break;
        case HTTPMethod::POST:
            res += "POST";
            break;
        case HTTPMethod::HEAD:
            res += "HEAD";
            break;
        default:
            break;
    }
    return res;
}
