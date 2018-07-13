#pragma mark -- JSONLexer

#include "JSONLexer.hpp"
#include <cctype>
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

std::shared_ptr<JSONToken> JSONLexer::getNextToken()
{
    std::shared_ptr<JSONToken> token = nullptr;
    
    while (!token)
    {
        auto ch = nextChar();
        if (ch == EOF)
        {
            break;
        }
        
        switch (state) 
        {
            case LexerState::Init:
                token = initState(ch);
                break;
            case LexerState::Number:
                token = numberState(ch);
                state = LexerState::Init;
                break;
            case LexerState::String:
                token = stringState(ch);
                state = LexerState::Init;
                break;
            case LexerState::Bool:
                token = booleanState(ch);
                state = LexerState::Init;
                break;
            case LexerState::Null:
                token = nullState(ch);
                state = LexerState::Init;
                break;
            default:
                break;
        }
    }
    
    return token;
}

std::shared_ptr<JSONToken> JSONLexer::initState(int16_t ch)
{
    std::shared_ptr<JSONToken> token = nullptr;
    
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
        token = std::make_shared<JSONToken>(TokenType::LeftBracket,"[");
    }
    else if (ch == ']')
    {
        token = std::make_shared<JSONToken>(TokenType::RightBracket,"]");
    }
    else if (ch == '{')
    {
        token = std::make_shared<JSONToken>(TokenType::LeftBrace,"{");
    }
    else if (ch == '}')
    {
        token = std::make_shared<JSONToken>(TokenType::RightBrace,"}");
    }
    else if (ch == ':')
    {
        token = std::make_shared<JSONToken>(TokenType::Colon,":");
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
        token = std::make_shared<JSONToken>(TokenType::Comma,",");
    }
    
    return token;
}

std::shared_ptr<JSONToken> JSONLexer::numberState(char ch)
{
    auto numberStr = std::string();
    if (isdigit(ch))
    {
        numberStr += ch;
    }

    while (1)
    {
        auto next_char = nextChar();

        if (isdigit(next_char) || next_char == '.')
        {
            numberStr += static_cast<char>(next_char);
        }
        else
        {
            if (next_char != EOF)
            {
                cache->push_back(next_char);
            }
            break;
        }
    }

    if (numberStr.empty())
    {
        return nullptr;
    }

    auto type = TokenType::Null;

    if (isInteger(numberStr))
    {
        type = TokenType::Integer;
    }
    else if (isFloat(numberStr))
    {
        type = TokenType::Float;
    }

    if (type == TokenType::Null)
    {
        throwError("except a number type's input");
        return nullptr;
    }

    if (ch == '-')
    {
        numberStr = ch + numberStr;
    }

    return std::make_shared<JSONToken>(type, numberStr);
}

bool JSONLexer::isInteger(std::string &content)
{
    auto size = content.size();
    if (size == 0)
    {
        return false;
    }

    auto firstChar = content[0];
    if (size == 1)
    {
        return isdigit(firstChar);
    }

    if (!(firstChar >= '1' && firstChar <= '9'))
    {
        return false;
    }

    auto result = true;
    for (auto i = 1; i < size; ++i)
    {
        if (!isdigit(content[i]))
        {
            result = false;
            break;
        }
    }

    return result;
}

bool JSONLexer::isFloat(std::string &content)
{
    auto size = content.size();
    if (size < 3)
    {
        return false;
    }

    auto index = content.rfind(".");
    if (index == std::string::npos)
    {
        return false;
    }

    auto integer = content.substr(0, index);
    if (!isInteger(integer))
    {
        return false;
    }

    auto decimal = content.substr(index + 1, size - index);
    auto result = true;

    for (auto ch : decimal)
    {
        if (!isdigit(ch))
        {
            result = false;
            break;
        }
    }

    return result;
}

std::shared_ptr<JSONToken> JSONLexer::stringState(int16_t ch)
{
    if (ch == '"')
    {
        return std::make_shared<JSONToken>(TokenType::String, "");
    }

    auto str = std::string();
    str += ch;

    while (1)
    {
        auto _ch = nextChar();
        if (_ch == EOF)
        {
            throwError("except \" at the end of string");
            return nullptr;
        }

        if (_ch == '"')
        { //is escape character
            auto count = 0;
            for (auto rite = str.rbegin(); rite != str.rend(); ++rite)
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

    return std::make_shared<JSONToken>(TokenType::String, str);
}

std::shared_ptr<JSONToken> JSONLexer::booleanState(int16_t ch)
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

    return std::make_shared<JSONToken>(TokenType::Boolean,std::move(boolVal));
}

std::shared_ptr<JSONToken> JSONLexer::nullState(int16_t ch)
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

    return std::make_shared<JSONToken>(TokenType::Null, std::move(nullStr));
}

#pragma mark -- JSONToken
JSONToken::JSONToken(TokenType _type, std::string _content)
{
    type = _type;
    content = _content;
}

bool JSONToken::isElementType()
{
    return type == TokenType::Null || type == TokenType::String || isNumberType();
}

bool JSONToken::isContainer()
{
    return type == TokenType::LeftBracket || type == TokenType::LeftBrace;
}

bool JSONToken::isNumberType()
{
    return type == TokenType::Integer || type == TokenType::Float || type == TokenType::Boolean;
}

bool JSONToken::isValueType()
{
    return isContainer() || isElementType();
}
