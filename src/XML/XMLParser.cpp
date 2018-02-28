#include "XMLParser.hpp"
#include "../Utility/Util.hpp"

XMLParser::XMLParser(std::string _input,InputType _type)
{
    tokenStack = std::stack<XMLTok *>();
    elementStack = std::stack<XMLDocument *>();
    lex = new XMLLex(_input,_type);
}

XMLParser::~XMLParser()
{
    if (lex)
    {
        delete lex;
        lex = nullptr;
    }
}

XMLTok * XMLParser::getNextToken()
{
    return lex?lex->getNextTok():nullptr;
}

XMLDocument * XMLParser::xmlTextToDocument()
{
    while (auto tok = getNextToken())
    {
        if (tok->content.empty())
        {
            continue;
        }
        
        switch (tok->type)
        {
            case TokType::FileAttribute:
                tokenStack.push(tok);
                break;
            case TokType::TagDeclare:
                tokenStack.push(tok);
                parse_tag_declare();
                break;
            case TokType::CData:
                tokenStack.push(tok);
                parse_cdata();
                break;
            case TokType::TagEnd:
                tokenStack.push(tok);
                parse_tag_end();
                break;
            case TokType::Content:
                tokenStack.push(tok);
                parse_content();
                break;
            default:
                break;
        }
    }
    
    return elementStack.empty()?nullptr:(elementStack.size() == 1?elementStack.top():nullptr);
}

void XMLParser::parse_content()
{
    auto tok = tokenStack.top();
    
    auto element = elementStack.top();
    element->setContent(tok->content);
    
    tokenStack.pop();
    
    delete tok;
    tok = nullptr;
}

void XMLParser::parse_tag_end()
{
    auto tok = tokenStack.top();
    tokenStack.pop();
    
    if (elementStack.empty())
    {
        throwError("empty element stack");
    }
    
    if (elementStack.top()->tagName != tok->content)
    {
        throwError("can not match tag start pattern");
    }
    
    delete tok;
    tok = nullptr;
    
    if (elementStack.size() == 1)
    {
        return;
    }
    
    auto subElement = elementStack.top();
    elementStack.pop();
    
    elementStack.top()->addChildNode(subElement);
}

void XMLParser::parse_cdata()
{
    auto tok = tokenStack.top();
    
    auto element = elementStack.top();
    element->setContent(tok->content);
    element->isCData = true;
    
    tokenStack.pop();
    
    delete tok;
    tok = nullptr;
}

void XMLParser::parse_tag_declare()
{
    auto tok = tokenStack.top();
    
    if (tok->type != TokType::TagDeclare)
    {
        throwError("token type error!");
    }
    
    auto name = parse_tag_name(tok->content);
    if (name.empty())
    {
        throwError("empty tag name");
    }
    auto element = new XMLDocument(name);
    elementStack.push(element);
    
    if (!tok->content.empty())
    {
        parse_tag_attr();
    }
    
    if (tok->isSelfClose)
    {
        elementStack.top()->isSelfClose = true;
        if (elementStack.size() > 1)
        {
            auto subElement = elementStack.top();
            elementStack.pop();
            elementStack.top()->addChildNode(subElement);
        }
    }
    
    tokenStack.pop();
    
    delete tok;
    tok = nullptr;
    
    if (!tokenStack.empty() &&
        tokenStack.top()->type == TokType::FileAttribute)
    {
        parse_file_attr();
    }
}

void XMLParser::parse_tag_attr()
{
    auto tok = tokenStack.top();
    auto attrStr = tok->content;
    auto root = elementStack.top();
    
    auto state = 1;
    auto buf = std::pair<std::string, std::string>();
    for (decltype(attrStr.size()) i = 0;i < attrStr.size();++i)
    {
        auto ch = attrStr[i];

        if (state == 1)
        {
            if (ch == '=')
            {
                if (buf.first.empty())
                {
                    throwError("empty attribute key at tag " + root->tagName);
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
                    buf.second = buf.second.substr(1,buf.second.size() - 2);
                    root->setAttribute(buf);
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
                    buf.second = buf.second.substr(1,buf.second.size() - 1);
                    root->setAttribute(buf);
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
}

std::string XMLParser::parse_tag_name(std::string &content)
{
    auto name = std::string();
    
    if (content.empty())
    {
        return name;
    }
    
    auto i = content.find(" ");
    if (i != std::string::npos)
    {
        auto j = content.find("=",i);
        if (j != std::string::npos)
        {
            if (j > i)
            {
                name = content.substr(0,i);
                content = content.substr(i + 1);
            }
            else
            {
                throwError("invalid tag declare format");
            }
        }
        else
        {
            name = content.substr(0,i);
            content = content.substr(i + 1);
        }
    }
    else
    {
        name = content;
        content.clear();
    }
    
    return name;
}

void XMLParser::parse_file_attr()
{
    auto tok = tokenStack.top();
    tokenStack.pop();
    
    auto element = elementStack.top();
    
    auto lexStr = tok->content;
    
    auto state = 1;
    auto buf = std::pair<std::string, std::string>();
    for (decltype(lexStr.size()) i = 0;i < lexStr.size();++i)
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
                    buf.second = buf.second.substr(1,buf.second.size() - 2);
                    element->setFileAttribute({buf.first,buf.second});
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
                    buf.second = buf.second.substr(1,buf.second.size() - 1);
                    element->setFileAttribute({buf.first,buf.second});
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
    
    delete tok;
    tok = nullptr;
}

