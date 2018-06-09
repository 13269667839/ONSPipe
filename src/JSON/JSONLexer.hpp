#ifndef JSONLexer_hpp
#define JSONLexer_hpp

#include <string>
#include <fstream>
#include <deque>
#include "../Utility/UtilConstant.hpp"

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

class JSONToken
{
public:
    TokenType type;
    std::string content;
public:
    JSONToken(TokenType _type = TokenType::Null,std::string _content = "");
    
    bool isElementType();
    bool isContainer();
};

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
    JSONToken * getNextToken();
private:
    int16_t nextChar();
    
    JSONToken * initState(int16_t ch);
    JSONToken * numberState(char ch);
    JSONToken * stringState(int16_t ch);
    JSONToken * booleanState(int16_t ch);
    JSONToken * nullState(int16_t ch);
private:
    InputType type;
    std::string content;
    
    std::ifstream *stream;
    std::string::size_type index;
    
    LexerState state;
    
    std::deque<int16_t> *cache;
};

#endif 
