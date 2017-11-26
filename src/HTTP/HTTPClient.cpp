#include "HTTPClient.hpp"
#include <thread>
#include "../Utility/HTTPParser.hpp"

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
    
    socket.sendAll(clientMsg);
    
    HTTPResponse *res = nullptr;
    auto parser = HTTPRecvMsgParser();
    while (1)
    {
        void *recvbuf = nullptr;
        long bytes = -2;
        
        std::tie(recvbuf,bytes) = socket.receive();
        
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
