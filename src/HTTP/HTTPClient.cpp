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

void HTTPClient::setRequestHeader(std::string key, std::string value)
{
    if (httpRequest)
    {
        httpRequest->addRequestHeader(key, value);
    }
}

void HTTPClient::sendMsg(TCPSocket &socket)
{
    auto clientMsg = httpRequest->toRequestMessage();
    if (clientMsg.empty())
    {
        throwError("http request message is null");
        return;
    }
    socket.sendAll(const_cast<char *>(clientMsg.c_str()), clientMsg.size());
}

std::unique_ptr<HTTPResponse> HTTPClient::recvMsg(TCPSocket &socket)
{
    HTTPResponse *res = nullptr;
    auto parser = HTTPRecvMsgParser();
    parser.method = methodStr();
    while (1)
    {
        auto recvbuf = socket.receive();
        parser.addToCache(recvbuf);
        if (recvbuf.empty() || parser.is_parse_msg())
        {
            res = parser.msg2res();
            break;
        }
    }
    return std::unique_ptr<HTTPResponse>(res);
}

std::unique_ptr<HTTPResponse> HTTPClient::requestByTCPSocket()
{
    auto host = url->host;
    auto family = AddressFamily::IPV4;
    auto port = std::to_string(url->portNumber);

    auto addressList = SocketConfig::getAddressInfo(host, port, family, SocketType::TCP);
    if (!addressList)
    {
        throwError("getAddressInfo() return null");
        return nullptr;
    }

    auto domain = addressList->ai_family;
    auto address = SocketConfig::netAddressToHostAddress(*(addressList->ai_addr));
    freeaddrinfo(addressList);
    addressList = nullptr;

    auto socket = TCPSocket(domain, family);
    setSocketConfig(socket);

    if (!socket.connect(address, url->portNumber))
    {
#ifdef DEBUG
        perror("connect() error : ");
#endif
        return nullptr;
    }

    sendMsg(socket);

    return recvMsg(socket);
}

std::unique_ptr<HTTPResponse> HTTPClient::request()
{
    if (https)
    {
    }
    return requestByTCPSocket();
}

void HTTPClient::setSocketConfig(TCPSocket &socket)
{
    //recv buff size
    int recvBufSize = 1024 * 10;
    socket.recvBuffSize = recvBufSize;
    if (!socket.setSocketOpt(SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize)))
    {
#ifdef DEBUG
        perror("set recv buff error : ");
#endif
    }

    //timeout
    struct timeval timeout = {.tv_sec = timeoutSeconds, .tv_usec = 0};
    if (!socket.setSocketOpt(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)))
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
    if (method == HTTPMethod::GET)
    {
        return "GET";
    }
    else if (method == HTTPMethod::POST)
    {
        return "POST";
    }
    else if (method == HTTPMethod::HEAD)
    {
        return "HEAD";
    }

    return std::string();
}
