#include "HTTPReqMsgParser.hpp"
#include <cctype>

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
    state = HTTPMessageParseState::Line;

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
    req.setRequestLine(method,path,version);
    
    for (auto kv : header)
    {
        req.addRequestHeader(kv.first,kv.second);
    }
    
    Util::byte u_str[cache->size()];
    std::copy(cache->begin(),cache->end(),u_str);
    req.body.assign(std::basic_string<Util::byte>(u_str,cache->size()));
}

bool HTTPReqMsgParser::parse_line()
{
    auto idx = -1;
    for (decltype(cache->size()) i = 0;i < cache->size() - 1;++i)
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
    for (auto i = 0;i < idx;++i)
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
    for (decltype(cache->size()) i = 0;i <= cache->size() - 4;++i)
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
    
    auto key = std::string();
    auto value = std::string();
    auto flag = 0;

    for (auto i = 0;i < idx;++i)
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
            if (content_length > 0) 
            {
                long size = cache->size();
                if (size >= content_length) 
                {
                    res = true;
                }
            }
            else 
            {
                res = true;
            }
        }
        break;
        default:
            break;
    }
    
    return res;
}

void HTTPReqMsgParser::addToCache(std::vector<Util::byte> &bytes)
{
    if (bytes.empty())
    {
        return;
    }

    if (!cache)
    {
        cache = new std::deque<Util::byte>();
    }

    cache->insert(cache->end(),std::begin(bytes),std::end(bytes));
}
