#ifndef XMLToken_hpp
#define XMLToken_hpp

//#define XML_PARSER_DEBUG

#include <string>

#ifdef XML_PARSER_DEBUG
    #include <ostream>
#endif

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

struct XMLTok
{
public:
    std::string content;
    TokType type;
    bool isSelfClose;
    
    XMLTok(std::string _content,TokType _type);
    
    #ifdef XML_PARSER_DEBUG
    friend std::ostream & operator << (std::ostream &os,XMLTok &tok);
    friend std::ostream & operator << (std::ostream &os,XMLTok *tok);
    #endif
private:
    #ifdef XML_PARSER_DEBUG
    std::string enumToStr();
    #endif
};

#endif
