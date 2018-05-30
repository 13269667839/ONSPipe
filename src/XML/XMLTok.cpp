#include "XMLTok.hpp"
#include <cctype>
#include "../Utility/UtilConstant.hpp"

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

#pragma mark -- XML Element Attributes
std::string XMLTok::tagName()
{
    auto name = std::string();

    if (content.empty())
    {
        return name;
    }

    if (type == TokType::TagDeclare)
    {
        auto idx = content.find(" ");
        if (idx == std::string::npos)
        {
            name += content;
        }
        else
        {
            name = content.substr(0, idx);
        }
    }
    else if (type == TokType::TagEnd)
    {
        name += content;
    }

    return name;
}

std::map<std::string, std::string> XMLTok::attribute()
{
    auto dic = std::map<std::string, std::string>();

    if (content.empty() || type != TokType::TagDeclare)
    {
        return dic;
    }

    auto name = tagName();
    auto nameSize = name.size();
    if (nameSize == content.size())
    {
        return dic;
    }

    auto attrStr = content.substr(nameSize);

    auto state = 1;
    auto buf = std::pair<std::string, std::string>();
    for (std::string::size_type i = 0; i < attrStr.size(); ++i)
    {
        auto ch = attrStr[i];

        if (state == 1)
        {
            if (ch == '=')
            {
                if (buf.first.empty())
                {
                    throwError("empty attribute key at tag " + name);
                }

                state = 2;
            }
            else
            {
                if (buf.first.empty())
                {
                    if (!isspace(ch))
                    {
                        buf.first += ch;
                    }
                }
                else
                {
                    buf.first += ch;
                }
            }
        }
        else if (state == 2)
        {
            if (isspace(ch))
            {
                auto firstCh = *begin(buf.second);
                auto lastCh = *(end(buf.second) - 1);

                auto isDoubleQuote = firstCh == '\"' && lastCh == '\"';
                auto isSingleQuote = firstCh == '\'' && lastCh == '\'';

                if (isDoubleQuote || isSingleQuote)
                {
                    buf.second = buf.second.substr(1, buf.second.size() - 2);
                    dic.insert(buf);
                    buf.first.clear();
                    buf.second.clear();
                    state = 1;
                }
                else if (i == attrStr.size() - 1)
                {
                    throwError("attribute value must inside of the quote");
                }
                else
                {
                    buf.second += ch;
                }
            }
            else if (i == attrStr.size() - 1)
            {
                auto firstCh = buf.second[0];

                auto isDoubleQuote = ch == '\"' && firstCh == '\"';
                auto isSingleQuote = ch == '\'' && firstCh == '\'';

                if (isDoubleQuote || isSingleQuote)
                {
                    buf.second = buf.second.substr(1, buf.second.size() - 1);
                    dic.insert(buf);
                    buf.first.clear();
                    buf.second.clear();
                    state = 1;
                }
                else
                {
                    throwError("attribute value must inside of the quote");
                }
            }
            else
            {
                if (buf.second.empty())
                {
                    if (ch == '\"' || ch == '\'')
                    {
                        buf.second += ch;
                    }
                }
                else
                {
                    buf.second += ch;
                }
            }
        }
    }

    return dic;
}

std::map<std::string, std::string> XMLTok::fileAttribute()
{
    auto dic = std::map<std::string, std::string>();

    if (content.empty() || type != TokType::FileAttribute)
    {
        return dic;
    }

    auto lexStr = content;

    auto state = 1;
    auto buf = std::pair<std::string, std::string>();
    for (std::string::size_type i = 0; i < lexStr.size(); ++i)
    {
        auto ch = lexStr[i];

        if (state == 1)
        {
            if (ch == '=')
            {
                if (buf.first.empty())
                {
                    throwError("empty attribute key at xml file attribute");
                }

                state = 2;
            }
            else
            {
                if (buf.first.empty())
                {
                    if (!isspace(ch))
                    {
                        buf.first += ch;
                    }
                }
                else
                {
                    buf.first += ch;
                }
            }
        }
        else if (state == 2)
        {
            if (isspace(ch))
            {
                if (*begin(buf.second) == '\"' && *(end(buf.second) - 1) == '\"')
                {
                    buf.second = buf.second.substr(1, buf.second.size() - 2);
                    dic.insert(buf);
                    buf.first.clear();
                    buf.second.clear();
                    state = 1;
                }
                else
                {
                    throwError("attribute value must inside of the quote");
                }
            }
            else if (i == lexStr.size() - 1)
            {
                if (ch == '\"' && *begin(buf.second) == '\"')
                {
                    buf.second = buf.second.substr(1, buf.second.size() - 1);
                    dic.insert(buf);
                    buf.first.clear();
                    buf.second.clear();
                    state = 1;
                }
                else
                {
                    throwError("attribute value must inside of the quote");
                }
            }
            else
            {
                if (buf.second.empty())
                {
                    if (ch == '\"')
                    {
                        buf.second += ch;
                    }
                }
                else
                {
                    buf.second += ch;
                }
            }
        }
    }

    return dic;
}