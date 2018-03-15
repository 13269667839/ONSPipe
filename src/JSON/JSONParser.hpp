#ifndef JSONParser_hpp
#define JSONParser_hpp

#include "JSONLexer.hpp"
#include "JSObject.hpp"

class JSONParser
{
public:
    JSONParser(InputType _type,std::string _content);
    ~JSONParser();
    
    JSObject * token2Object();
private:
    JSONToken * nextToken();
    JSString * elementObject(JSONToken *tok);
    JSArray * arrayObject();
    JSMap * mapObject();
    
    JSONLexer *lex;
};

#endif
