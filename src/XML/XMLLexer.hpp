#ifndef XMLLexer_hpp
#define XMLLexer_hpp

#include "XMLToken.hpp"
#include <fstream>

#define LEX_COMMENT

enum class InputType : int
{
    FileName = 0,
    XMLText
};

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
    std::string::size_type *idx;
    
    TokType state;
    
    TokType lastTokType;
private:
    int16_t getNextChar();
    
    void initState(int16_t ch,std::string &localCache);
    void tagStartState(int16_t ch,std::string &localCache);
    XMLTok * commentState(int16_t ch,std::string &localCache);
    XMLTok * fileAttributeState(int16_t ch,std::string &localCache);
    XMLTok * tagEndState(int16_t ch,std::string &localCache);
    XMLTok * tagDeclareState(int16_t ch,std::string &localCache);
    XMLTok * contentState(int16_t ch,std::string &localCache);
    XMLTok * docTypeState(int16_t ch,std::string &localCache);
    XMLTok * CDataState(int16_t ch,std::string &localCache);
};

#endif
