#ifndef JSONLexer_hpp
#define JSONLexer_hpp

#include <fstream>
#include <deque>
#include "../Utility/UtilConstant.hpp"
#include "JSONToken.hpp"

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
