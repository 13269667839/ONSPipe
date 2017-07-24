#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
{
    header = nullptr;
    
    initParameter();
}

HTTPRequest::~HTTPRequest()
{
    if (header)
    {
        header->clear();
        delete header;
        header = nullptr;
    }
}

void HTTPRequest::initParameter()
{
    parseState = HTTPMessageParseState::Init;
    recvHTTPReqMsgBuf = std::string();
    content_length = -1;
    
    HTTPMethod = "GET";
    path = "/";
    httpVersion = "HTTP/1.1";
    
    if (header)
    {
        header->clear();
    }
    else
    {
        header = nullptr;
    }
    
    query = std::string();
}

void HTTPRequest::addRequestHeader(std::pair<std::string,std::string> pair)
{
    if (!header)
    {
        header = new std::map<std::string,std::string>();
    }
    
    if (!pair.first.empty() && !pair.second.empty())
    {
        header->insert(pair);
    }
}

std::string HTTPRequest::toRequestMessage()
{
    return requestLine() + "\r\n" + requestHeader() + "\r\n\r\n" + query;
}

std::string HTTPRequest::requestLine()
{
    if (HTTPMethod.empty() || httpVersion.empty() || path.empty())
    {
        Util::throwError("invalid http request line format");
    }
    
    return HTTPMethod + " " + path + " " + httpVersion;
}

std::string HTTPRequest::requestHeader()
{
    if (!header || header->empty())
    {
        Util::throwError("invalid http request header format");
        return "";
    }
    
    auto arr = std::vector<std::string>();
    for (auto pair : *header)
    {
        if (!pair.first.empty() && !pair.second.empty())
        {
            arr.push_back(pair.first + ": " + pair.second);
        }
    }
    
    return Util::join(arr, "\r\n");

}

std::ostream & operator << (std::ostream &os,HTTPRequest *res)
{
    os<<*res;
    return os;
}

std::ostream & operator << (std::ostream &os,HTTPRequest& res)
{
    os<<res.toRequestMessage();
    return os;
}

bool HTTPRequest::parseRequestMessage(std::string reqMsg)
{
    bool res = false;
    
    if (reqMsg.empty())
    {
        res = true;
    }
    else
    {
        recvHTTPReqMsgBuf += reqMsg;
        
        if (parseState == HTTPMessageParseState::Init)
        {
            auto idx = recvHTTPReqMsgBuf.find("\r\n");
            if (idx != std::string::npos)
            {
                parseState = HTTPMessageParseState::Line;
                goto ParseLine;
            }
        }
        else if (parseState == HTTPMessageParseState::Line)
        {
        ParseLine:
            auto idx = recvHTTPReqMsgBuf.find("\r\n");
            if (idx != std::string::npos)
            {
                auto line = recvHTTPReqMsgBuf.substr(0,idx);
                
                auto arr = Util::split(line, " ");
                if (arr.size() == 3)
                {
                    HTTPMethod = arr[0];
                    path = arr[1];
                    httpVersion = arr[2];
                }
                else
                {
                    Util::throwError("invalid http request line format");
                }
                
                recvHTTPReqMsgBuf = recvHTTPReqMsgBuf.substr(idx + 2);
                parseState = HTTPMessageParseState::Header;
                
                if (recvHTTPReqMsgBuf.find("\r\n\r\n") != std::string::npos)
                {
                    goto ParseHeader;
                }
            }
        }
        else if (parseState == HTTPMessageParseState::Header)
        {
        ParseHeader:
            auto idx = recvHTTPReqMsgBuf.find("\r\n\r\n");
            if (idx != std::string::npos)
            {
                auto head = recvHTTPReqMsgBuf.substr(0,idx);
                
                auto arr = Util::split(head, "\r\n");
                if (!arr.empty())
                {
                    for (auto item : arr)
                    {
                        auto pair = Util::split(item, ": ");
                        if (pair.size() == 2)
                        {
                            addRequestHeader({Util::toLowerStr(pair[0]),pair[1]});
                        }
                    }
                    
                    auto ite = header->find("content-length");
                    if (ite != end(*header))
                    {
                        content_length = std::atol(ite->second.c_str());
                    }
                }
                else
                {
                    Util::throwError("invalid http request header format");
                }
                
                recvHTTPReqMsgBuf = recvHTTPReqMsgBuf.substr(idx + 4);
                parseState = HTTPMessageParseState::Body;
                
                if (!recvHTTPReqMsgBuf.empty() && content_length > 0)
                {
                    goto ParseBody;
                }
                else
                {
                    res = true;
                }
            }
        }
        else if (parseState == HTTPMessageParseState::Body)
        {
        ParseBody:
            query += recvHTTPReqMsgBuf;
            recvHTTPReqMsgBuf.clear();
            if (content_length != -1 && query.size() >= content_length)
            {
                res = true;
            }
        }
    }
    
    return res;
}
