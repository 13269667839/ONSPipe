#include "JSONParser.hpp"

JSONParser::JSONParser(InputType _type,std::string _content)
{
    lex = new JSONLexer(_type,_content);
}

JSONParser::~JSONParser()
{
    if (lex)
    {
        delete lex;
        lex = nullptr;
    }
}

JSONToken * JSONParser::nextToken()
{
    return lex->getNextToken();
}

JSObject * JSONParser::elementObject(JSONToken *tok)
{
    if (!tok) 
    {
        return nullptr;
    }

    if (tok->isNumberType())
    {
        return new JSNumber(tok->content,tok->type);
    }
    else if (tok->type == TokenType::String)
    {
        return new JSString(tok->content,tok->type);
    }
    else if (tok->type == TokenType::Null)
    {
        return new JSObject();
    }

    return nullptr;
}

JSArray * JSONParser::arrayObject()
{
    JSArray *arr = nullptr;
    
    while (1)
    {
        auto tok = nextToken();
        if (!tok)
        {
            throwError("except ] before array end");
            break;
        }
        else if (tok->type == TokenType::RightBracket)
        {
            delete tok;
            break;
        }

        if (!arr)
        {
            arr = new JSArray();
        }

        JSObject *obj = nullptr;
        if (tok->isElementType())
        {
            obj = elementObject(tok);
        }
        else if (tok->type == TokenType::LeftBracket)
        {
            obj = arrayObject();
        }
        else if (tok->type == TokenType::LeftBrace)
        {
            obj = mapObject();
        }

        arr->addObject(obj);

        delete tok;
    }
    
    return arr;
}

JSMap * JSONParser::mapObject()
{
    JSMap *map = nullptr;
    std::string key;
    int flag = 0;
    
    while (1)
    {
        auto tok = nextToken();
        if (!tok)
        {
            throwError("except } before map end");
            break;
        }
        else if (tok->type == TokenType::RightBrace)
        {
            delete tok;
            break;
        }

        if (!map)
        {
            map = new JSMap();
        }

        if (flag == 0)
        {
            if (tok->type != TokenType::String)
            {
                throwError("key of a map must be string type");
                break;
            }

            flag = 1;
            key = std::move(tok->content);
            delete tok;
        }
        else if (flag == 1)
        {
            if (tok->type != TokenType::Colon)
            {
                throwError("miss : between key and value");
                break;
            }

            flag = 2;
            delete tok;
        }
        else if (flag == 2)
        {
            if (tok->isElementType() || tok->isContainer())
            {
                if (key.empty())
                {
                    throwError("key is empty");
                }
                else
                {
                    JSObject *obj = nullptr;
                    if (tok->isElementType())
                    {
                        obj = elementObject(tok);
                    }
                    else if (tok->type == TokenType::LeftBracket)
                    {
                        obj = arrayObject();
                    }
                    else if (tok->type == TokenType::LeftBrace)
                    {
                        obj = mapObject();
                    }

                    map->setObjectAndKey(key, obj);
                    delete tok;
                }
            }
            else if (tok->type == TokenType::Comma)
            {
                flag = 0;
                key.clear();
                delete tok;
            }
            else
            {
                throwError("unexcept token at map parser function");
            }
        }
    }
    
    return map;
}

std::unique_ptr<JSObject> JSONParser::token2Object()
{
    if (!lex)
    {
        throwError("lexer is null");
        return nullptr;
    }
    
    JSObject *obj = nullptr;
    
    while (1)
    {
        auto tok = nextToken();
        if (!tok)
        {
            break;
        }
        
        if (tok->isElementType())
        {
            obj = elementObject(tok);
            delete tok;
        }
        else if (tok->type == TokenType::LeftBrace)
        {
            delete tok;
            obj = mapObject();
        }
        else if (tok->type == TokenType::LeftBracket)
        {
            delete tok;
            obj = arrayObject();
        }
        else 
        {
            delete tok;
            throwError("unexcept token");
        }
    }
    
    return std::unique_ptr<JSObject>(obj);
}
