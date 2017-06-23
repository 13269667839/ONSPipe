#ifndef JSONLexer_hpp
#define JSONLexer_hpp

#include <string>
#include <fstream>
#include <deque>

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

enum class SourceType
{
    File = 0,
    Text
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
    JSONLexer(SourceType _type,std::string _content);
    ~JSONLexer();
public:
    JSONToken * getNextToken();
private:
    int nextChar();
    
    JSONToken * initState(int ch);
    JSONToken * numberState(char ch);
    JSONToken * stringState(char ch);
    JSONToken * booleanState(char ch);
    JSONToken * nullState();
private:
    SourceType type;
    std::string content;
    
    std::ifstream *stream;
    std::string::size_type *index;
    
    LexerState state;
    
    std::deque<int> *cache;
};

#endif 
