#include "JSONParser.hpp"
#include "../Utility/Utility.hpp"

JSONParser::JSONParser()
{
    lex = nullptr;
}

JSONParser::~JSONParser()
{
    if (lex)
    {
        delete lex;
    }
}

JSONToken * JSONParser::nextToken()
{
    return lex?lex->getNextToken():nullptr;
}

JSString * JSONParser::elementObject(JSONToken *tok)
{
    return new JSString(tok->content,tok->type);
}

JSArray * JSONParser::arrayObject()
{
    JSArray *arr = nullptr;
    
    while (1)
    {
        auto tok = nextToken();
        if (!tok)
        {
            Utility::throwError("except ] before array end");
        }
        else if (tok->type == TokenType::RightBracket)
        {
            delete tok;
            break;
        }
        else
        {
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
            
            if (obj)
            {
                arr->addObject(obj);
            }
            
            delete tok;
        }
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
            Utility::throwError("except } before map end");
        }
        else if (tok->type == TokenType::RightBrace)
        {
            delete tok;
            break;
        }
        else
        {
            if (!map)
            {
                map = new JSMap();
            }
            
            if (flag == 0)
            {
                if (tok->type == TokenType::String)
                {
                    flag = 1;
                    key = tok->content;
                    delete tok;
                }
                else
                {
                    Utility::throwError("key must be string type");
                }
            }
            else if (flag == 1)
            {
                if (tok->type == TokenType::Colon)
                {
                    flag = 2;
                    delete tok;
                }
                else
                {
                    Utility::throwError("miss : between key and value");
                }
            }
            else if (flag == 2)
            {
                if (tok->isElementType() || tok->isContainer())
                {
                    if (key.empty())
                    {
                        Utility::throwError("key is empty");
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
                        
                        if (obj)
                        {
                            map->setObjectAndKey(key, obj);
                        }
                        
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
                    Utility::throwError("unexcept token at map parser function");
                }
            }
        }
    }
    
    return map;
}

JSObject * JSONParser::token2Object()
{
    if (!lex)
    {
        Utility::throwError("lexer is null");
    }
    
    JSObject *obj = nullptr;
    
    while (1)
    {
        auto tok = nextToken();
        if (!tok)
        {
            break;
        }
        else if (obj && tok)
        {
            Utility::throwError("unexcept token");
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
    }
    
    return obj;
}
