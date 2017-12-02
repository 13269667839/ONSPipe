#ifndef XMLLexer_hpp
#define XMLLexer_hpp

#include <string>
#include <ostream>
#include <fstream>

enum class TokType : int
{
    Init = 0,
    Comment,
    FileAttribute,
    TagStart,
    TagDeclare,
    TagEnd,
    Content,
    DocType,
    CData
};
    
enum class InputType : int
{
    File = 0,
    Text
};
    
struct XMLTok
{
public:
    std::string content;
    TokType type;
    bool isSelfClose;
        
    XMLTok(std::string _content,TokType _type);
    
    friend std::ostream & operator << (std::ostream &os,const XMLTok &tok);
    friend std::ostream & operator << (std::ostream &os,const XMLTok *tok);
private:
    std::string type2Str() const;
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
