#include "XMLTok.hpp"

XMLTok::XMLTok(std::string _content,TokType _type)
{
    content = _content;
    type = _type;
    isSelfClose = false;
}

std::ostream & operator << (std::ostream &os,const XMLTok &tok)
{
    os<<"type    : "<<tok.type2Str()<<std::endl;
    os<<"content : "<<tok.content;
    return os;
}

std::ostream & operator << (std::ostream &os,const XMLTok *tok)
{
    if (tok)
    {
        os<<*tok;
    }
    return os;
}

std::string XMLTok::type2Str() const
{
    auto str = std::string();
    
    switch (type)
    {
        case TokType::Init:
            str.assign("Init");
            break;
        case TokType::TagStart:
            str.assign("TagStart");
            break;
        case TokType::Comment:
            str.assign("Comment");
            break;
        case TokType::FileAttribute:
            str.assign("FileAttribute");
            break;
        case TokType::TagDeclare:
            str.assign("TagDeclare");
            break;
        case TokType::TagEnd:
            str.assign("TagEnd");
            break;
        case TokType::Content:
            str.assign("Content");
            break;
        case TokType::DocType:
            str.assign("DocType");
            break;
        case TokType::CData:
            str.assign("CData");
            break;
        default:
            break;
    }
    
    return str;
}
