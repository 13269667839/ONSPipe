#include "HTTPResponse.hpp"
#include "../Utility/Util.hpp"
#include <cctype>

HTTPResponse::HTTPResponse()
{
    httpVersion = std::string();
    statusCode = 0;
    reason = std::string();
    header = nullptr;
    responseBody = std::string();
    
    state = HTTPMessageParseState::Init;
    recvBuf = std::string();
    content_length = -1;
    isChunk = false;
    recvMsgLen = 0;
}

HTTPResponse::~HTTPResponse()
{
    if (header)
    {
        header->clear();
        delete header;
        header = nullptr;
    }
}

std::ostream & operator << (std::ostream &os,HTTPResponse *res)
{
    if (res)
    {
        os<<*res;
    }
    return os;
}

std::ostream & operator << (std::ostream &os,HTTPResponse &res)
{
    os<<res.toResponseMessage();
    return os;
}

std::string HTTPResponse::toResponseMessage()
{
    auto line = httpVersion + " " + std::to_string(statusCode) + " " + reason;
    
    auto head = std::string();
    if (header && !header->empty())
    {
        auto arr = std::vector<std::string>();
        for (auto pair : *header)
        {
            arr.push_back(pair.first + ": " + pair.second);
        }
        head = Util::join(arr, "\r\n");
    }

    return line + "\r\n" + head + "\r\n\r\n" + responseBody + "\r\n";
}

void HTTPResponse::addResponseHead(std::pair<std::string,std::string> pair)
{
    if (!pair.first.empty() && !pair.second.empty())
    {
        if (!header)
        {
            header = new std::map<std::string,std::string>();
        }
        
        header->insert(pair);
    }
}

bool HTTPResponse::initState()
{
    auto idx = recvBuf.find("\r\n");
    if (idx == std::string::npos)
    {
        Util::throwError("invalid http response line format");
    }
    state = HTTPMessageParseState::Line;
    return true;
}

bool HTTPResponse::bodyState()
{
    responseBody += recvBuf;
    recvBuf.clear();

    bool res = false;
    if (content_length > 0)
    {
        if (recvMsgLen >= content_length)
        {
            res = true;
        }
    }
    else if (isChunk)
    {
        auto idx = responseBody.rfind("\r\n0\r\n\r\n");
        if (idx != std::string::npos)
        {
            responseBody = responseBody.substr(0,idx);
            removeChunkBodyMsg();
            res = true;
        }
    }
    
    return res;
}

void HTTPResponse::removeChunkBodyMsg()
{
    auto idx = responseBody.find("\r\n");
    if (idx == std::string::npos || idx + 2 >= responseBody.size())
    {
        return;
    }
    
    auto hexStr = responseBody.substr(0,idx);
    for (auto ch : hexStr)
    {
        if (!ishexnumber(ch))
        {
            return;
        }
    }
    
    responseBody = responseBody.substr(idx + 2);
}

bool HTTPResponse::headerState()
{
    auto idx = recvBuf.find("\r\n\r\n");
    bool res = false;
    if (idx != std::string::npos)
    {
        res = true;
        
        auto headStr = recvBuf.substr(0,idx);
        recvBuf = recvBuf.substr(idx + 4);
        state = HTTPMessageParseState::Body;
        
        auto arr = Util::split(headStr, "\r\n");
        if (!arr.empty())
        {
            for (auto item : arr)
            {
                auto pair = Util::split(item, ": ");
                
                if (pair.size() >= 2)
                {
                    auto key = pair[0];
                    auto value = pair[1];
                    for (int i = 2;i < pair.size();++i)
                    {
                        value += pair[i];
                    }
                    
                    if (!key.empty() && !value.empty())
                    {
                        addResponseHead({Util::toLowerStr(key),value});
                    }
                }
                else
                {
                    Util::throwError("invalid http response head format");
                }
            }
            
            auto ite = header->find("content-length");
            if (ite != end(*header))
            {
                content_length = atol(ite->second.c_str());
            }
            else
            {
                ite = header->find("transfer-encoding");
                if (ite != end(*header))
                {
                    auto val = Util::toLowerStr(ite->second);
                    if (val == "chunked")
                    {
                        isChunk = true;
                    }
                }
            }
        }
        else
        {
            Util::throwError("invalid http response head format");
        }
    }
    
    return res;
}

bool HTTPResponse::lineState()
{
    auto idx = recvBuf.find("\r\n");
    auto line = recvBuf.substr(0,idx);
    recvBuf = recvBuf.substr(idx + 2);
    state = HTTPMessageParseState::Body;
    
    auto arr = Util::split(line, " ");
    if (arr.size() >= 3)
    {
        httpVersion = arr[0];
        statusCode = atoi(arr[1].c_str());
        for (int i = 2;i < arr.size();++i)
        {
            reason += arr[i];
        }
    }
    else
    {
        Util::throwError("invalid http response line format");
    }
    
    return recvBuf.find("\r\n\r\n") != std::string::npos;
}

bool HTTPResponse::parseHttpResponseMsg(std::string msg)
{
    bool res = false;
    
    if (msg.empty())
    {
        res = true;
    }
    else
    {
        recvBuf += msg;
        recvMsgLen += msg.size();
        
        if (state == HTTPMessageParseState::Init)
        {
            if (initState())
            {
                goto ParseLine;
            }
        }
        else if (state == HTTPMessageParseState::Line)
        {
        ParseLine:
            if (lineState())
            {
                goto ParseHeader;
            }
        }
        else if (state == HTTPMessageParseState::Header)
        {
        ParseHeader:
            if (headerState())
            {
                if (content_length > 0 || isChunk)
                {
                    goto ParseBody;
                }
                else
                {
                    if (recvBuf.empty())
                    {
                        res = true;
                    }
                }
            }
        }
        else if (state == HTTPMessageParseState::Body)
        {
        ParseBody:
            res = bodyState();
        }
    }
    
    return res;
}
