#ifndef JSONLexer_hpp
#define JSONLexer_hpp

#pragma mark -- JSONToken

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

#pragma mark -- JSONLexer

#include <fstream>
#include <deque>
#include <regex>
#include <memory>
#include "../Utility/UtilConstant.hpp"

enum class LexerState
{
    Init = 0,
    Number,
    String,
    Bool,
    Null
};

class JSONLexer
{
public:
    JSONLexer(InputType _type,std::string _content);
    ~JSONLexer();
public:
    std::shared_ptr<JSONToken> getNextToken();
private:
    char nextChar();
    
    std::shared_ptr<JSONToken> initState(char ch);
    std::shared_ptr<JSONToken> numberState(char ch);
    std::shared_ptr<JSONToken> stringState(char ch);
    std::shared_ptr<JSONToken> booleanState(char ch);
    std::shared_ptr<JSONToken> nullState(char ch);

    bool isInteger(std::string &content);
    bool isFloat(std::string &content);
private:
    InputType type;
    std::string content;
    
    std::ifstream *stream;
    std::string::size_type index;
    std::string::size_type contentLength;
    
    LexerState state;
    
    std::deque<char> *cache;
};

#endif 
