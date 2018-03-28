#include "HTTPClient.hpp"
#include "../Utility/HTTPRecvMsgParser.hpp"

#ifdef DEBUG 
    #include <cerrno>
    #include <cstdio>
#endif


HTTPClient::HTTPClient(std::string _url, HTTPMethod _method)
{
    url = nullptr;
    httpRequest = nullptr;
    https = false;
    timeoutSeconds = 10;

    if (!_url.empty())
    {
        method = _method;
        url = new URL(_url);
        https = url->scheme == "https";
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

void HTTPClient::sendMsg(Socket &socket)
{
    auto clientMsg = httpRequest->toRequestMessage();
    if (clientMsg.empty())
    {
        throwError("http request message is null");
        return;
    }
    socket.sendAll(const_cast<char *>(clientMsg.c_str()), clientMsg.size(), https);
}

HTTPResponse * HTTPClient::recvMsg(Socket &socket)
{
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
    return res;
}

std::unique_ptr<HTTPResponse> HTTPClient::request()
{
    checkParams();
    
    auto socket = Socket(url->host, url->portNumber);
    
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

    sendMsg(socket);

    HTTPResponse *res = recvMsg(socket);
    return std::unique_ptr<HTTPResponse>(res);
}

void HTTPClient::setSocketConfig(Socket &socket)
{
    //recv buff size
    int recvBufSize = 1024 * 10;
    socket.recvBuffSize = recvBufSize;
    if (socket.setSocketOpt(SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize)) == -1)
    {
#ifdef DEBUG
        perror("set recv buff error : ");
#endif
    }

    //timeout
    struct timeval timeout = {.tv_sec = timeoutSeconds, .tv_usec = 0};
    if (socket.setSocketOpt(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1)
    {
#ifdef DEBUG
        perror("set timeout error : ");
#endif
    }
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
