#include "XMLLexer.hpp"
#include <cctype>

#pragma mark -- XMLTok
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

#pragma mark -- XMLLex
XMLLex::XMLLex(std::string _input,InputType _type)
{
    source = _input;
    type = _type;
    
    stream = nullptr;
    idx = nullptr;
    state = TokType::Init;
    lastTokType = TokType::Init;
    localCache = std::string();
    
    if (type == InputType::File)
    {
        if (source.empty())
        {
            throwError("file name is nil");
        }
        
        stream = new std::ifstream(source);
        if (!stream->is_open())
        {
            throwError("file open error");
        }
    }
    else if (type == InputType::Text)
    {
        if (source.empty())
        {
            throwError("xml text is null");
        }
        
        idx = new std::string::size_type(0);
    }
}

XMLLex::~XMLLex()
{
    if (stream)
    {
        if (stream->is_open())
        {
            stream->close();
        }
        delete stream;
        stream = nullptr;
    }
    
    if (idx)
    {
        delete idx;
        idx = nullptr;
    }
}

int16_t XMLLex::getNextChar()
{
    int16_t ch = EOF;
    
    if (type == InputType::File)
    {
        ch = stream->get();
    }
    else if (type == InputType::Text)
    {
        ch = source[*idx];
        (*idx)++;
    }
    
    return ch;
}

XMLTok * XMLLex::getNextTok()
{
    XMLTok *tok = nullptr;
    localCache.clear();
    auto ch = EOF;
    
    while ((ch = getNextChar()) != EOF)
    {
        switch (state)
        {
            case TokType::Init:
                initState(ch);
                break;
            case TokType::TagStart:
                tagStartState(ch);
                break;
            case TokType::Comment:
                tok = commentState(ch);
                break;
            case TokType::FileAttribute:
                tok = fileAttributeState(ch);
                break;
            case TokType::TagDeclare:
                tok = tagDeclareState(ch);
                break;
            case TokType::TagEnd:
                tok = tagEndState(ch);
                break;
            case TokType::Content:
                tok = contentState(ch);
                break;
            case TokType::DocType:
                tok = docTypeState(ch);
                break;
            case TokType::CData:
                tok = CDataState(ch);
                break;
            default:
                break;
        }
        
        if (tok)
        {
            lastTokType = tok->type;
            break;
        }
    }
    
    return tok;
}

XMLTok * XMLLex::docTypeState(int16_t ch)
{
    XMLTok *tok = nullptr;
    
    if (ch == '>')
    {
        tok = new XMLTok(localCache,TokType::DocType);
        state = TokType::Init;
    }
    else
    {
        localCache += ch;
    }
    
    return tok;
}

void XMLLex::initState(int16_t ch)
{
    if (ch == '<')
    {
        state = TokType::TagStart;
    }
    else if (lastTokType == TokType::TagDeclare)
    {
        if (ch > 32)
        {
            localCache.clear();
            state = TokType::Content;
            localCache += ch;
        }
    }
}

void XMLLex::tagStartState(int16_t ch)
{
    if (ch == '!' && localCache.empty())
    {
        localCache += ch;
    }
    else if (localCache[0] == '!')
    {
        if (ch == '-')
        {
            localCache += ch;
            if (localCache.size() == 3)
            {
                if (localCache == "!--")
                {
                    localCache.clear();
                    state = TokType::Comment;
                }
                else
                {
                    throwError("error on comment");
                }
            }
        }
        else
        {
            localCache += ch;
            if (localCache.size() == 8)
            {
                if (localCache == "!DOCTYPE" || localCache == "!doctype")
                {
                    state = TokType::DocType;
                    localCache.clear();
                }
                else if (localCache == "![CDATA[")
                {
                    state = TokType::CData;
                    localCache.clear();
                }
                else
                {
                    throwError("xml doctype error");
                }
            }
        }
    }
    else if (ch == '?')
    {
        localCache.clear();
        state = TokType::FileAttribute;
    }
    else if (ch == '/')
    {
        localCache.clear();
        state = TokType::TagEnd;
    }
    else
    {
        localCache.clear();
        localCache += ch;
        state = TokType::TagDeclare;
    }
}

XMLTok * XMLLex::CDataState(int16_t ch)
{
    XMLTok *tok = nullptr;
    
    if (ch == '>' && localCache.size() > 3)
    {
        auto idx = localCache.rfind("]]");
        if (idx == localCache.size() - 2)
        {
            tok = new XMLTok(localCache.substr(0,idx),TokType::CData);
            state = TokType::Init;
        }
    }
    else
    {
        localCache += ch;
    }
    
    return tok;
}

XMLTok * XMLLex::commentState(int16_t ch)
{
    XMLTok *tok = nullptr;
    
    localCache += ch;
    
    if (ch == '>' && localCache.size() >= 3)
    {
        auto buf = std::string();
        
        for (auto i = localCache.size() - 1;i > 0;--i)
        {
            char ite = localCache[i];
            
            if (!(ite == '-' || ite == '>' || isspace(ite)))
            {
                break;
            }
            
            if (!isspace(ite))
            {
                buf += ite;
                if (buf == ">--")
                {
                    auto content = localCache.substr(0,i);
                    tok = new XMLTok(content,TokType::Comment);
                    state = TokType::Init;
                    break;
                }
            }
        }
    }
    
    return tok;
}

XMLTok * XMLLex::fileAttributeState(int16_t ch)
{
    XMLTok *tok = nullptr;
    
    localCache += ch;
    
    if (ch == '>' && localCache.size() >= 2)
    {
        for (auto i = localCache.size() - 2;i > 0;--i)
        {
            if (localCache[i] == '?')
            {
                auto content = localCache.substr(4,i - 4);
                tok = new XMLTok(content,TokType::FileAttribute);
                state = TokType::Init;
                break;
            }
        }
    }
    
    return tok;
}

XMLTok * XMLLex::tagDeclareState(int16_t ch)
{
    XMLTok *tok = nullptr;
    
    if (ch == '>')
    {
        auto i = localCache.rfind('/');
        
        if (i != std::string::npos)
        {
            for (auto j = i + 1;j < localCache.size();++j)
            {
                if (!isspace(localCache[j]))
                {
                    i = std::string::npos;
                    break;
                }
            }
        }
        
        if (i == std::string::npos)
        {
            tok = new XMLTok(localCache,TokType::TagDeclare);
        }
        else
        {
            tok = new XMLTok(localCache.substr(0,i),TokType::TagDeclare);
            tok->isSelfClose = true;
        }
        state = TokType::Init;
    }
    else
    {
        localCache += ch;
    }
    
    return tok;
}

XMLTok * XMLLex::tagEndState(int16_t ch)
{
    XMLTok *tok = nullptr;
    
    if (ch == '>')
    {
        tok = new XMLTok(localCache,TokType::TagEnd);
        state = TokType::Init;
    }
    else
    {
        localCache += ch;
    }
    
    return tok;
}

XMLTok * XMLLex::contentState(int16_t ch)
{
    XMLTok *tok = nullptr;
    
    if (ch == '<')
    {
        Util::trimRight(localCache, ' ');
        tok = new XMLTok(localCache,TokType::Content);
        state = TokType::TagStart;
    }
    else if (ch >= 32)
    {
        localCache += ch;
    }
    
    return tok;
}
