#include "JSONLexer.hpp"
#include <cctype>
#include <regex>
#include "../Utility/Util.hpp"

#pragma mark -- JSONToken
JSONToken::JSONToken(TokenType _type,std::string _content)
{
    type = _type;
    content = _content;
}

bool JSONToken::isElementType()
{
    return type == TokenType::Null || type == TokenType::String || type == TokenType::Integer || type == TokenType::Float || type == TokenType::Boolean;
}

bool JSONToken::isContainer()
{
    return type == TokenType::LeftBracket || type == TokenType::LeftBrace;
}

#pragma mark -- JSONLexer
JSONLexer::JSONLexer(SourceType _type,std::string _content)
{
    if (_content.empty())
    {
        throwError("input is empty");
    }
    
    type = _type;
    content = _content;
    stream = nullptr;
    index = nullptr;
    state = LexerState::Init;
    cache = new std::deque<int16_t>();
    
    if (type == SourceType::File)
    {
        stream = new std::ifstream(_content);
        if (!stream->is_open())
        {
            delete stream;
            throwError("file open error");
        }
    }
    else
    {
        index = new std::string::size_type(0);
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
    
    if (index)
    {
        delete index;
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
        if (type == SourceType::File)
        {
            if (stream)
            {
                ch = stream->get();
            }
        }
        else if (type == SourceType::Text)
        {
            if (index && *index < content.length())
            {
                ch = content[(*index)++];
            }
        }
    }
    return ch;
}

JSONToken * JSONLexer::getNextToken()
{
    int ch = EOF;
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
            tok = nullState();
            state = LexerState::Init;
        }
        
        if (tok)
        {
            break;
        }
    }
    
    return tok;
}

JSONToken * JSONLexer::initState(int ch)
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
    else if (isnumber(ch))
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
        if (isnumber(_ch))
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
        if (isnumber(tmp) || tmp == '.')
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
        
        if (_ch == '"' && str[str.size() - 1] != '\\')
        {
            break;
        }
        
        str += _ch;
    }

    return new JSONToken(TokenType::String,str);
}

JSONToken * JSONLexer::booleanState(char ch)
{
    auto str = std::string();
    str += ch;
    
    int count = 0;
    if (ch == 't')
    {
        count = 3;
    }
    else if (ch == 'f')
    {
        count = 4;
    }
    
    while (count > 0)
    {
        char _ch = nextChar();
        if (_ch == EOF)
        {
            throwError("unexcept input");
            break;
        }
        else
        {
            str += _ch;
            count--;
        }
    }
    
    return (str == "true" || str == "false")?new JSONToken(TokenType::Boolean,str):nullptr;
}

JSONToken * JSONLexer::nullState()
{
    auto len = 3;
    char str[3];
    auto ch = nextChar();

    while (len > 0 && ch != EOF)
    {
        str[3 - len] = ch;
        --len;
        ch = nextChar();
    }
    
    if (strncmp(str,"ull",3) != 0)
    {
        throwError("invalid literal null");
        return nullptr;
    }
    
    return new JSONToken(TokenType::Null,"null");
}
