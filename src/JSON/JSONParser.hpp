#ifndef JSONParser_hpp
#define JSONParser_hpp

#include <memory>

#include "JSONLexer.hpp"
#include "JSObject.hpp"

class JSONParser
{
public:
    JSONParser(InputType _type,std::string _content);
    ~JSONParser();
    
    std::unique_ptr<JSObject> token2Object();
private:
    JSONToken * nextToken();
    JSString * elementObject(JSONToken *tok);
    JSArray * arrayObject();
    JSMap * mapObject();
    
    JSONLexer *lex;
};

#endif
