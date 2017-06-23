#include "XMLToken.hpp"

XMLTok::XMLTok(std::string _content,TokType _type)
{
    content = _content;
    type = _type;
    isSelfClose = false;
}

#ifdef XML_PARSER_DEBUG
std::string XMLTok::enumToStr()
{
    auto str = std::string();
    
    if (type == TokType::Init)
    {
        str = "Init";
    }
    else if (type == TokType::TagStart)
    {
        str = "TagStart";
    }
    else if (type == TokType::Comment)
    {
        str = "Comment";
    }
    else if (type == TokType::FileAttribute)
    {
        str = "FileAttribute";
    }
    else if (type == TokType::TagDeclare)
    {
        str = "TagDeclare";
    }
    else if (type == TokType::TagEnd)
    {
        str = "TagEnd";
    }
    else if (type == TokType::Content)
    {
        str = "Content";
    }
    else if (type == TokType::DocType)
    {
        str = "DocType";
    }
    else if (type == TokType::CData)
    {
        str = "CDATA";
    }
    
    return str;
}

std::ostream & operator << (std::ostream &os,XMLTok &tok)
{
    os<<tok.enumToStr()<<" : "<<tok.content;
    return os;
}

std::ostream & operator << (std::ostream &os,XMLTok *tok)
{
    if (tok)
    {
        os<<*tok;
    }
    return os;
}
#endif
