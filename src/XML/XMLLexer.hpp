#ifndef XMLLexer_hpp
#define XMLLexer_hpp

#include <fstream>
#include "../Utility/Util.hpp"
#include "XMLTok.hpp"

class XMLLex
{
public:
    XMLLex(std::string _input,InputType _type);
    ~XMLLex();
    
    XMLTok * getNextTok();
private:
    std::string source;
    InputType type;
    
    std::ifstream *stream;
    std::string::size_type idx;
    
    TokType state;
    
    TokType lastTokType;
    std::string localCache;
private:
    int16_t getNextChar();
    
    void initState(int16_t ch);
    void tagStartState(int16_t ch);
    XMLTok * commentState(int16_t ch);
    XMLTok * fileAttributeState(int16_t ch);
    XMLTok * tagEndState(int16_t ch);
    XMLTok * tagDeclareState(int16_t ch);
    XMLTok * contentState(int16_t ch);
    XMLTok * docTypeState(int16_t ch);
    XMLTok * CDataState(int16_t ch);
};

#endif
