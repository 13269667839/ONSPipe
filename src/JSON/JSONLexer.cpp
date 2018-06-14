#include "JSONLexer.hpp"
#include <cctype>
#include <regex>
#include <cstring>

JSONLexer::JSONLexer(InputType _type,std::string _content)
{
    if (_content.empty())
    {
        throwError("input is empty");
    }
    
    type = _type;
    content = _content;
    stream = nullptr;
    index = 0;
    state = LexerState::Init;
    cache = new std::deque<int16_t>();
    
    if (type == InputType::File)
    {
        stream = new std::ifstream(_content);
        if (!stream->is_open())
        {
            delete stream;
            throwError("file open error");
        }
    }
}

JSONLexer::~JSONLexer()
{
    if (stream)
    {
        if (stream->is_open())
        {
            stream->close();
        }
        delete stream;
    }
    
    if (cache)
    {
        delete cache;
    }
}

int16_t JSONLexer::nextChar()
{
    if (cache && !cache->empty())
    {
        auto ch = cache->at(0);
        cache->pop_front();
        return ch;
    }

    if (type == InputType::File)
    {
        return stream->get();
    }
    
    if (type == InputType::Text)
    {
        if (index < content.length())
        {
            auto ch = content[index];
            index += 1;
            return ch;
        }
    }

    return EOF;
}

JSONToken * JSONLexer::getNextToken()
{
    JSONToken *tok = nullptr;
    
    while (!tok)
    {
        auto ch = nextChar();
        if (ch == EOF)
        {
            break;
        }
        
        switch (state) 
        {
            case LexerState::Init:
                tok = initState(ch);
                break;
            case LexerState::Number:
                tok = numberState(ch);
                state = LexerState::Init;
                break;
            case LexerState::String:
                tok = stringState(ch);
                state = LexerState::Init;
                break;
            case LexerState::Bool:
                tok = booleanState(ch);
                state = LexerState::Init;
                break;
            case LexerState::Null:
                tok = nullState(ch);
                state = LexerState::Init;
                break;
            default:
                break;
        }
    }
    
    return tok;
}

JSONToken * JSONLexer::initState(int16_t ch)
{
    JSONToken *tok = nullptr;
    
    if (ch == '"')
    {
        state = LexerState::String;
    }
    else if (ch == 't' || ch == 'f')
    {
        state = LexerState::Bool;
        cache->push_back(ch);
    }
    else if (ch == '[')
    {
        tok = new JSONToken(TokenType::LeftBracket,"[");
    }
    else if (ch == ']')
    {
        tok = new JSONToken(TokenType::RightBracket,"]");
    }
    else if (ch == '{')
    {
        tok = new JSONToken(TokenType::LeftBrace,"{");
    }
    else if (ch == '}')
    {
        tok = new JSONToken(TokenType::RightBrace,"}");
    }
    else if (ch == ':')
    {
        tok = new JSONToken(TokenType::Colon,":");
    }
    else if (isdigit(ch))
    {
        state = LexerState::Number;
        cache->push_back(ch);
    }
    else if (ch == 'n')
    {
        state = LexerState::Null;
    }
    else if (ch == '-')
    {
        char _ch = nextChar();
        if (isdigit(_ch))
        {
            cache->push_back(ch);
            cache->push_back(_ch);
            state = LexerState::Number;
        }
        else
        {
            throwError("unexcept input behind -");
        }
    }
    else if (ch == ',')
    {
        tok = new JSONToken(TokenType::Comma,",");
    }
    
    return tok;
}

JSONToken * JSONLexer::numberState(char ch)
{
    std::string numberStr = std::string();
    if (ch != '-')
    {
        numberStr += ch;
    }
    bool isDot = false;
    
    while (1)
    {
        char tmp = nextChar();
        if (isdigit(tmp) || tmp == '.')
        {
            numberStr += tmp;
            if (tmp == '.')
            {
                isDot = true;
            }
        }
        else
        {
            if (tmp != EOF)
            {
                cache->push_back(tmp);
            }
            break;
        }
    }
    
    JSONToken *tok = nullptr;
    std::string convertStr;
    if (isDot && *(numberStr.end() - 1) != '.')
    {
        double doubleVal = atof(numberStr.c_str());
        if (ch == '-')
        {
            doubleVal = -doubleVal;
        }
        convertStr = std::to_string(doubleVal);
    }
    else
    {
        long longVal = atol(numberStr.c_str());
        if (ch == '-')
        {
            longVal = -longVal;
        }
        convertStr = std::to_string(longVal);
    }
    
    if (!convertStr.empty())
    {
        auto integerReg = std::regex("([1-9][0-9]{1,})|([0-9])");
        auto floatReg = std::regex("(([1-9][0-9]{1,})|([0-9])).[0-9]*");
        if (std::regex_match(convertStr,integerReg))
        {
            tok = new JSONToken(TokenType::Integer,convertStr);
        }
        else if (std::regex_match(convertStr,floatReg))
        {
            tok = new JSONToken(TokenType::Float,convertStr);
        }
    }
    
    return tok;
}

JSONToken * JSONLexer::stringState(int16_t ch)
{
    if (ch == EOF)
    {
        throwError("excepr \" at the end of string");
        return nullptr;
    }
    else if (ch == '"')
    {
        return new JSONToken(TokenType::String,"");
    }

    auto str = std::string();
    str += ch;

    while (1)
    {
        auto _ch = nextChar();
        if (_ch == EOF)
        {
            throwError("excepr \" at the end of string");
            return nullptr;
        }

        if (_ch == '"')
        {
            auto count = 0;
            for (auto rite = str.rbegin();rite != str.rend();++rite)
            {
                if (*rite == '\\')
                {
                    count++;
                }
                else 
                {
                    break;
                }
            }
            if (count % 2 == 0)
            {
                break;
            }
        }
        
        str += _ch;
    }

    return new JSONToken(TokenType::String,str);
}

JSONToken * JSONLexer::booleanState(int16_t ch)
{
    auto length = (ch == 't')?3:((ch == 'f')?4:0);
    if (length == 0) 
    {
        throwError("except 't' or 'f' at the boolean state");
        return nullptr;
    }

    auto boolVal = std::string();
    boolVal += ch;
    
    for (auto i = 0;i < length;++i)
    {
        auto _ch = nextChar();
        if (_ch == EOF)
        {
            break;
        }
        boolVal += _ch;
    }

    if (boolVal != "true" && boolVal != "false") 
    {
        throwError("invalid boolean literal");
        return nullptr;
    }

    return new JSONToken(TokenType::Boolean,std::move(boolVal));
}

JSONToken * JSONLexer::nullState(int16_t ch)
{
    auto nullStr = std::string("n");
    nullStr += ch;

    auto count = 2;
    while (count > 0)
    {
        auto _ch = nextChar();
        if (_ch == EOF)
        {
            break;
        }

        nullStr += _ch;
        count--;
    }

    if (nullStr != "null")
    {
        throwError("invalid literal null");
        return nullptr;
    }

    return new JSONToken(TokenType::Null, std::move(nullStr));
}
