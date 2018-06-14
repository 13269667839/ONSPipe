#ifndef JSONToken_hpp
#define JSONToken_hpp

enum class TokenType
{
    Null = 0,
    String,
    Integer,
    Float,
    Boolean,
    Array,
    Map,
    ///冒号
    Colon,
    ///左花括号
    LeftBrace,
    ///右花括号
    RightBrace,
    ///左中括号
    LeftBracket,
    ///右中括号
    RightBracket,
    ///逗号
    Comma
};

#include <string>

class JSONToken
{
public:
    TokenType type;
    std::string content;
public:
    JSONToken(TokenType _type = TokenType::Null,std::string _content = "");
    
    bool isElementType();
    bool isContainer();
    bool isNumberType();
    bool isValueType();
};

#endif