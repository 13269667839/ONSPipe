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
    int16_t ch = EOF;
    
    if (cache && !cache->empty())
    {
        ch = *(std::begin(*cache));
        cache->pop_front();
    }
    else
    {
        if (type == InputType::File)
        {
            if (stream)
            {
                ch = stream->get();
            }
        }
        else if (type == InputType::Text)
        {
            if (index < content.length())
            {
                ch = content[index++];
            }
        }
    }
    return ch;
}

JSONToken * JSONLexer::getNextToken()
{
    int16_t ch = EOF;
    JSONToken *tok = nullptr;
    
    while (1)
    {
        ch = nextChar();
        if (ch == EOF)
        {
            break;
        }
        
        if (state == LexerState::Init)
        {
            tok = initState(ch);
        }
        else if (state == LexerState::Number)
        {
            tok = numberState(ch);
            state = LexerState::Init;
        }
        else if (state == LexerState::String)
        {
            tok = stringState(ch);
            state = LexerState::Init;
        }
        else if (state == LexerState::Bool)
        {
            tok = booleanState(ch);
            state = LexerState::Init;
        }
        else if (state == LexerState::Null)
        {
            tok = nullState(ch);
            state = LexerState::Init;
        }
        
        if (tok)
        {
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
    char str[4] = {'\0','\0','\0','\0'};
    const int len = ch == 't'?3:4;
    
    for (auto i = 0;i < len;++i)
    {
        auto _ch = nextChar();
        if (_ch == EOF)
        {
            break;
        }
        str[i] = _ch;
    }

    auto isTrue = ch == 't' && strncmp(str,"rue",3) == 0;
    auto isFalse = ch == 'f' && strncmp(str,"alse",4) == 0;

    if (!isTrue && !isFalse)
    {
        throwError("invalid boolean literal");
        return nullptr;
    }

    return new JSONToken(TokenType::Boolean,isTrue?"true":"false");
}

JSONToken * JSONLexer::nullState(int16_t ch)
{
    auto len = 2;
    char str[3];
    str[0] = ch;

    while (len > 0)
    {
        auto _ch = nextChar();
        if (_ch == EOF)
        {
            break;
        }
        str[3 - len] = _ch;
        len--;
    }
    
    if (strncmp(str,"ull",3) != 0)
    {
        throwError("invalid literal null");
        return nullptr;
    }
    
    return new JSONToken(TokenType::Null,"null");
}
