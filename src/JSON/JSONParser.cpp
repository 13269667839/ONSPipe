#include "JSONParser.hpp"

JSONParser::JSONParser(InputType _type, std::string _content)
{
    lexer = std::make_shared<JSONLexer>(_type, _content);
}

std::shared_ptr<JSObject> JSONParser::tokenToJSObject()
{
    if (!lexer)
    {
        throwError("lexer is nullptr");
        return nullptr;
    }

    auto token = lexer->getNextToken();
    return parserMainLoop(token);
}

std::shared_ptr<JSObject> JSONParser::parserMainLoop(std::shared_ptr<JSONToken> &token)
{
    std::shared_ptr<JSObject> ptr = nullptr;

    if (!token)
    {
        return ptr;
    }

    switch (token->type)
    {
    case TokenType::LeftBrace:
        ptr = tokenToJSMap();
        break;
    case TokenType::LeftBracket:
        ptr = tokenToJSArray();
        break;
    case TokenType::Null:
    case TokenType::String:
    case TokenType::Integer:
    case TokenType::Float:
    case TokenType::Boolean:
        ptr = tokenToPrimitive(token);
        break;
    default:
        throwError("unexcept token");
        break;
    }

    return ptr;
}

std::shared_ptr<JSObject> JSONParser::tokenToPrimitive(std::shared_ptr<JSONToken> &token)
{
    std::shared_ptr<JSObject> ptr = nullptr;

    switch (token->type)
    {
    case TokenType::Integer:
    case TokenType::Float:
    case TokenType::Boolean:
        ptr = std::make_shared<JSNumber>(token->content, token->type);
        break;
    case TokenType::Null:
        ptr = std::make_shared<JSObject>();
        break;
    case TokenType::String:
        ptr = std::make_shared<JSString>(token->content);
        break;
    default:
        break;
    }

    return ptr;
}

std::shared_ptr<JSArray> JSONParser::tokenToJSArray()
{
    std::shared_ptr<JSArray> arrayRef = nullptr;

    auto token = lexer->getNextToken();
    while (token && token->type != TokenType::RightBracket)
    {
        if (token->isValueType())
        {
            if (!arrayRef)
            {
                arrayRef = std::make_shared<JSArray>();
            }

            auto object = parserMainLoop(token);
            arrayRef->addObject(object);
        }

        token = lexer->getNextToken();
    }

    return arrayRef;
}

std::shared_ptr<JSMap> JSONParser::tokenToJSMap()
{
    std::shared_ptr<JSMap> mapRef = nullptr;

    auto key = std::string();
    auto flag = 0;
    auto token = lexer->getNextToken();
    while (token && token->type != TokenType::RightBrace)
    {
        if (flag == 0) //key
        {
            if (token->type != TokenType::String)
            {
                throwError("key of a map must be string type");
                break;
            }

            if (token->content.empty())
            {
                throwError("key is empty of a pair in map");
                break;
            }

            flag = 1;
            key = std::move(token->content);
        }
        else if (flag == 1) //:
        {
            if (token->type != TokenType::Colon)
            {
                throwError("miss : between key and value");
                break;
            }

            flag = 2;
        }
        else if (flag == 2) //value
        {
            if (token->type == TokenType::Comma)
            {
                flag = 0;
                key.clear();
            }
            else
            {
                auto value = parserMainLoop(token);

                if (!mapRef)
                {
                    mapRef = std::make_shared<JSMap>();
                }
                mapRef->setObjectAndKey(key, value);
            }
        }

        token = lexer->getNextToken();
    }

    return mapRef;
}
