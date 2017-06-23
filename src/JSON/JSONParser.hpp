#ifndef JSONParser_hpp
#define JSONParser_hpp

#include "JSONLexer.hpp"
#include "JSObject.hpp"

class JSONParser
{
public:
    JSONParser();
    ~JSONParser();
    
    JSObject * token2Object();
public:
    JSONLexer *lex;
private:
    JSONToken * nextToken();
    JSString * elementObject(JSONToken *tok);
    JSArray * arrayObject();
    JSMap * mapObject();
};

#endif
