#ifndef XMLTok_hpp
#define XMLTok_hpp

#include <ostream>
#include "../Utility/Util.hpp"

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
public:
    std::string tagName();
    std::map<std::string,std::string> attribute();
    std::map<std::string,std::string> fileAttribute();
private:
    std::string type2Str() const;
};

#endif