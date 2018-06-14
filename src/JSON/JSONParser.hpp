#ifndef JSONParser_hpp
#define JSONParser_hpp

#include <memory>

#include "JSONLexer.hpp"
#include "JSObject.hpp"

class JSONParser
{
public:
    JSONParser(InputType _type,std::string _content);
public:
    std::shared_ptr<JSObject> tokenToJSObject();
private:
    std::shared_ptr<JSONToken> nextToken();

    std::shared_ptr<JSObject>   tokenToPrimitive(std::shared_ptr<JSONToken> &token);
    std::shared_ptr<JSArray>    tokenToJSArray();
    std::shared_ptr<JSMap>      tokenToJSMap();

    std::shared_ptr<JSObject> parserMainLoop(std::shared_ptr<JSONToken> &token);
private:
    std::shared_ptr<JSONLexer> lexer;
};

#endif
