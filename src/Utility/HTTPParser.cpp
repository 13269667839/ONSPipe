#include "HTTPParser.hpp"

#pragma mark -- HTTPRecvMsgParser
HTTPRecvMsgParser::~HTTPRecvMsgParser()
{
    if (cache)
    {
        cache->clear();
        delete cache;
        cache = nullptr;
    }
}

HTTPRecvMsgParser::HTTPRecvMsgParser()
{
    state = HTTPMessageParseState::Line;
    cache = nullptr;
    version = std::string();
    status_code = std::string();
    reason = std::string();
    header = std::map<std::string,std::string>();
    content_length = -1;
    isChunk = false;
}

bool HTTPRecvMsgParser::is_parse_msg()
{
    auto res = false;
    
    if (!cache || cache->empty())
    {
        return res;
    }
    
    if (state == HTTPMessageParseState::Line)
    {
        if (parse_line())
        {
            state = HTTPMessageParseState::Header;
            if (!cache->empty())
            {
                goto Header;
            }
        }
    }
    else if (state == HTTPMessageParseState::Header)
    {
    Header:
        if (parse_header())
        {
            state = HTTPMessageParseState::Body;
            if (!cache->empty())
            {
                goto Body;
            }
        }
    }
    else if (state == HTTPMessageParseState::Body)
    {
    Body:
        res = parse_body();
    }
    
    return res;
}

bool HTTPRecvMsgParser::parse_body()
{
    auto res = false;
    
    if (content_length != -1)
    {
        res = cache->size() >= content_length;
    }
    else if (isChunk)
    {
        long idx = -1;
        long i = cache->size() - 1;
        for (;i >= 0;--i)
        {
            if (cache->size() - i >= 7)
            {
                if (cache->at(i)     == Util::byte('\r') &&
                    cache->at(i + 1) == Util::byte('\n') &&
                    cache->at(i + 2) == Util::byte('0')  &&
                    cache->at(i + 3) == Util::byte('\r') &&
                    cache->at(i + 4) == Util::byte('\n') &&
                    cache->at(i + 5) == Util::byte('\r') &&
                    cache->at(i + 6) == Util::byte('\n'))
                {
                    idx = i;
                    break;
                }
            }
        }
        
        if (idx != -1)
        {
            auto count = cache->size() - idx;
            while (count > 0)
            {
                cache->pop_back();
                count--;
            }
            
            i = 0;
            for (;i < cache->size() - 1;++i)
            {
                if (cache->at(i)     == Util::byte('\r') &&
                    cache->at(i + 1) == Util::byte('\n'))
                {
                    auto isHex = true;
                    auto hexStr = std::string(cache->begin(),cache->begin() + i);
                    for (auto ch : hexStr)
                    {
                        if (!ishexnumber(ch))
                        {
                            isHex = false;
                        }
                    }
                    
                    if (isHex)
                    {
                        auto j = i + 2;
                        while (j > 0)
                        {
                            cache->pop_front();
                            j--;
                        }
                    }
                    
                    break;
                }
            }
            
            res = true;
        }
    }
    
    return res;
}

bool HTTPRecvMsgParser::parse_header()
{
    auto res = false;
    
    auto idx = -1;
    auto i = 0;
    for (;i < cache->size() - 4;++i)
    {
        if (cache->at(i)     == Util::byte('\r') &&
            cache->at(i + 1) == Util::byte('\n') &&
            cache->at(i + 2) == Util::byte('\r') &&
            cache->at(i + 3) == Util::byte('\n'))
        {
            idx = i;
            break;
        }
    }
    
    if (idx != -1)
    {
        i = 0;
        auto key = std::string();
        auto value = std::string();
        auto flag = 0;
        
        for (;i < idx;++i)
        {
            auto ch = cache->at(0);
            cache->pop_front();
            
            if (ch           == Util::byte('\r') &&
                cache->at(0) == Util::byte('\n'))
            {
                header[key] = value;
                
                key.clear();
                value.clear();
                
                flag = 0;
                
                i++;
                cache->pop_front();
                continue;
            }
            
            if (flag == 0)
            {
                if (ch           == Util::byte(':') &&
                    cache->at(0) == Util::byte(' '))
                {
                    flag = 1;
                    i++;
                    cache->pop_front();
                }
                else
                {
                    key += ch;
                }
            }
            else if (flag == 1)
            {
                value += ch;
            }
        }
        
        //pop \r\n\r\n
        cache->pop_front();
        cache->pop_front();
        cache->pop_front();
        cache->pop_front();
        
        res = true;
        
        auto ite = header.find("Content-Length");
        if (ite != header.end())
        {
            content_length = atol(ite->second.c_str());
        }
        else
        {
            ite = header.find("Transfer-Encoding");
            if (ite != header.end())
            {
                isChunk = ite->second == "chunked";
            }
        }
    }
    
    return res;
}

bool HTTPRecvMsgParser::parse_line()
{
    auto res = false;
    
    auto idx = -1;
    auto i = 0;
    for (;i < cache->size() - 1;++i)
    {
        if (cache->at(i)     == Util::byte('\r') &&
            cache->at(i + 1) == Util::byte('\n'))
        {
            idx = i;
            break;
        }
    }
    
    if (idx != -1)
    {
        auto space_count = 0;
        i = 0;
        for (;i < idx;++i)
        {
            auto ch = cache->at(0);
            cache->pop_front();
            
            if (isspace(ch) && space_count < 2)
            {
                space_count++;
                continue;
            }
            
            if (space_count == 0)
            {
                version += ch;
            }
            else if (space_count == 1)
            {
                status_code += ch;
            }
            else if (space_count == 2)
            {
                reason += ch;
            }
        }
        
        //pop \r\n
        cache->pop_front();
        cache->pop_front();
        
        res = true;
    }
    
    return res;
}

HTTPResponse * HTTPRecvMsgParser::msg2res()
{
    auto res = new HTTPResponse();
    
    res->version = version;
    res->statusCode = atoi(status_code.c_str());
    res->reason = reason;
    
    for (auto kv : header)
    {
        res->addResponseHead(kv);
    }
    
    if (!cache->empty())
    {
        res->responseBody = std::string(cache->begin(),cache->end());
    }
    
    return res;
}

#pragma mark -- HTTPReqMsgParser
HTTPReqMsgParser::HTTPReqMsgParser()
{
    cache = nullptr;
    
    state = HTTPMessageParseState::Line;
    content_length = -1;
    
    version = std::string();
    path = std::string();
    method = std::string();
    
    header = std::map<std::string,std::string>();
}

void HTTPReqMsgParser::initParams()
{
    version.clear();
    path.clear();
    method.clear();
    
    header.clear();
    
    if (cache)
    {
        cache->clear();
    }
}

HTTPReqMsgParser::~HTTPReqMsgParser()
{
    if (cache)
    {
        cache->clear();
        delete cache;
        cache = nullptr;
    }
}

void HTTPReqMsgParser::msg2req(HTTPRequest &req)
{
    req.method = method;
    req.path = path;
    req.version = version;
    
    for (auto kv : header)
    {
        req.addRequestHeader(kv);
    }
    
    req.requestBody = std::string(cache->begin(),cache->end());
}

bool HTTPReqMsgParser::parse_line()
{
    auto idx = -1;
    auto i = 0;
    for (;i < cache->size() - 1;++i)
    {
        if (cache->at(i)     == Util::byte('\r') &&
            cache->at(i + 1) == Util::byte('\n'))
        {
            idx = i;
            break;
        }
    }
    
    if (idx == -1)
    {
        return false;
    }
    
    auto space_count = 0;
    i = 0;
    for (;i < idx;++i)
    {
        auto ch = cache->at(0);
        cache->pop_front();

        if (isspace(ch) && space_count < 2)
        {
            space_count++;
            continue;
        }

        if (space_count == 0)
        {
            method += ch;
        }
        else if (space_count == 1)
        {
            path += ch;
        }
        else if (space_count == 2)
        {
            version += ch;
        }
    }

    //pop \r\n
    cache->pop_front();
    cache->pop_front();
    
    return true;
}

bool HTTPReqMsgParser::parse_header()
{
    auto idx = -1;
    auto i = 0;
    for (;i <= cache->size() - 4;++i)
    {
        if (cache->at(i)     == Util::byte('\r') &&
            cache->at(i + 1) == Util::byte('\n') &&
            cache->at(i + 2) == Util::byte('\r') &&
            cache->at(i + 3) == Util::byte('\n'))
        {
            idx = i;
            break;
        }
    }
    
    if (idx == -1)
    {
        return false;
    }
    
    i = 0;
    auto key = std::string();
    auto value = std::string();
    auto flag = 0;

    for (;i < idx;++i)
    {
        auto ch = cache->at(0);
        cache->pop_front();

        if (ch           == Util::byte('\r') &&
            cache->at(0) == Util::byte('\n'))
        {
            header[key] = value;

            key.clear();
            value.clear();

            flag = 0;

            i++;
            cache->pop_front();
            continue;
        }

        if (flag == 0)
        {
            if (ch           == Util::byte(':') &&
                cache->at(0) == Util::byte(' '))
            {
                flag = 1;
                i++;
                cache->pop_front();
            }
            else
            {
                key += ch;
            }
        }
        else if (flag == 1)
        {
            value += ch;
        }
    }

    //pop \r\n\r\n
    cache->pop_front();
    cache->pop_front();
    cache->pop_front();
    cache->pop_front();
    
    content_length = atol(header["Content-Length"].c_str());
    
    return true;
}

bool HTTPReqMsgParser::is_parse_msg()
{
    auto res = false;
    
    if (!cache || cache->empty())
    {
        return true;
    }
    
    switch (state)
    {
        case HTTPMessageParseState::Line:
        {
            if (parse_line())
            {
                state = HTTPMessageParseState::Header;
                if (!cache->empty())
                {
                    goto Header;
                }
            }
        }
        break;
        case HTTPMessageParseState::Header:
        {
        Header:
            if (parse_header())
            {
                state = HTTPMessageParseState::Body;
                goto Body;
            }
        }
        break;
        case HTTPMessageParseState::Body:
        {
        Body:
            res = cache->size() >= content_length;
        }
        break;
        default:
            break;
    }
    
    return res;
}
