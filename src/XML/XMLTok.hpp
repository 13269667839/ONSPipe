#ifndef XMLTok_hpp
#define XMLTok_hpp

#include <ostream>
#include <string>

enum class TokType
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
    
    friend std::ostream & operator << (std::ostream &os,const XMLTok &tok);
    friend std::ostream & operator << (std::ostream &os,const XMLTok *tok);
private:
    std::string type2Str() const;
};

#endif