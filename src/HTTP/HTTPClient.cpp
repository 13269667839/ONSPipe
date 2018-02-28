#include "HTTPClient.hpp"
#include <thread>
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

void HTTPClient::asyncRequest(RequestCallback callback)
{
    if (!callback)
    {
        return;
    }
    
    auto thread_main = [](HTTPClient *client,RequestCallback callback)
    {
        HTTPResponse *res = nullptr;
        auto err = std::string();
        
        try
        {
            res = client->syncRequest();
        }
        catch (std::logic_error error)
        {
            err = error.what();
        }
        
        if (callback)
        {
            callback(res,err);
        }
        
        if (res)
        {
            delete res;
            res = nullptr;
        }
    };
    
    std::thread(thread_main,this,callback).detach();
}

HTTPResponse * HTTPClient::syncRequest()
{
    if (!httpRequest)
    {
        throwError("http request is not set");
        return nullptr;
    }
    else if (!url || url->path.empty())
    {
        throwError("url is null");
        return nullptr;
    }
    
    auto socket = Socket(url->host, url->portNumber);
    if (!socket.connect())
    {
        throwError("can not connect to server");
    }
    
    setSocketConfig(socket);

    auto https = url->scheme == "https";
    
    if (https)
    {
        socket.ssl_config();
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
        void *recvbuf = nullptr;
        long bytes = -2;
        
        if (https)
        {
            std::tie(recvbuf,bytes) = socket.ssl_read();
        }
        else 
        {
            std::tie(recvbuf,bytes) = socket.receive();
        }
        
        if (!recvbuf || bytes == -2)
        {
            res = parser.msg2res();
            break;
        }
        
        auto u_buf = static_cast<Util::byte *>(recvbuf);
        if (!u_buf)
        {
            break;
        }
        
        if (!parser.cache)
        {
            parser.cache = new std::deque<Util::byte>();
        }
        
        for (auto i = 0;i < bytes;++i)
        {
            parser.cache->push_back(u_buf[i]);
        }
        
        delete u_buf;
        u_buf = nullptr;
        
        if (parser.is_parse_msg())
        {
            res = parser.msg2res();
            break;
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
    setRequestHeader("accept-encoding","gzip, deflate, br");
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
