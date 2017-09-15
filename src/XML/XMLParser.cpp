#include "XMLParser.hpp"
#include "../Utility/Util.hpp"

XMLParser::XMLParser(std::string _input,InputType _type)
{
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

std::string XMLParser::parseTagName(std::string &lexStr)
{
    auto name = std::string();
    
    if (!lexStr.empty())
    {
        auto i = lexStr.find(" ");
        if (i != std::string::npos)
        {
            auto j = lexStr.find("=",i);
            if (j != std::string::npos)
            {
                if (j > i)
                {
                    name = lexStr.substr(0,i);
                    lexStr = lexStr.substr(i + 1);
                }
                else
                {
                    Util::throwError("invalid tag declare format");
                }
            }
            else
            {
                name = lexStr.substr(0,i);
                lexStr = lexStr.substr(i + 1);
            }
        }
        else
        {
            name = lexStr;
        }
    }
    
    return name;
}

void XMLParser::parseTagAttribute(std::string &attrStr,XMLDocument *root)
{
    if (attrStr.empty() || !root)
    {
        return;
    }
    
    int state = 1;
    auto buf = std::pair<std::string, std::string>();
    for (auto i = 0;i < attrStr.size();++i)
    {
        auto ch = attrStr[i];
        
        if (state == 1)
        {
            if (ch == '=')
            {
                if (buf.first.empty())
                {
                    Util::throwError("empty attribute key at tag " + root->tagName);
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
                    Util::throwError("attribute value must inside of the quote");
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
                    Util::throwError("attribute value must inside of the quote");
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

XMLDocument * XMLParser::parseTagStart(std::string lexStr,bool isSelfClose)
{
    auto name = parseTagName(lexStr);
    
    XMLDocument *root = nullptr;
    if (!name.empty())
    {
        root = new XMLDocument(name);
        parseTagAttribute(lexStr, root);
        
        if (!isSelfClose)
        {
            while (1)
            {
                auto tok = getNextToken();
                
                if (!tok || tok->content.empty())
                {
                    break;
                }
                
                if (tok->type == TokType::TagDeclare)
                {
                    auto child = parseTagStart(tok->content,tok->isSelfClose);
                    if (child)
                    {
                        root->addChildNode(child);
                    }
                }
                else if (tok->type == TokType::TagEnd)
                {
                    if (tok->content != name)
                    {
                        Util::throwError("the end tag must have the same name to begin tag");
                    }
                    break;
                }
                else if (tok->type == TokType::Content || tok->type == TokType::CData)
                {
                    root->setContent(tok->content);
                    if (tok->type == TokType::CData)
                    {
                        root->isCData = true;
                    }
                }
            }
        }
        else
        {
            root->isSelfClose = true;
        }
    }
    
    return root;
}

std::vector<std::pair<std::string, std::string>> XMLParser::parseFileAttribute(std::string &lexStr)
{
    auto attr = std::vector<std::pair<std::string, std::string>>();
    
    if (lexStr.empty())
    {
        return attr;
    }
    
    int state = 1;
    auto buf = std::pair<std::string, std::string>();
    for (auto i = 0;i < lexStr.size();++i)
    {
        auto ch = lexStr[i];
        
        if (state == 1)
        {
            if (ch == '=')
            {
                if (buf.first.empty())
                {
                    Util::throwError("empty attribute key at xml file attribute");
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
                    attr.push_back(buf);
                    buf.first.clear();
                    buf.second.clear();
                    state = 1;
                }
                else
                {
                    Util::throwError("attribute value must inside of the quote");
                }
            }
            else if (i == lexStr.size() - 1)
            {
                if (ch == '\"' && *begin(buf.second) == '\"')
                {
                    buf.second = buf.second.substr(1,buf.second.size() - 1);
                    attr.push_back(buf);
                    buf.first.clear();
                    buf.second.clear();
                    state = 1;
                }
                else
                {
                    Util::throwError("attribute value must inside of the quote");
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
    
    return attr;
}

XMLDocument * XMLParser::xmlTextToDocument()
{
    XMLDocument *root = nullptr;
    auto fileAttr = std::vector<std::pair<std::string, std::string>>();
    
    while (1)
    {
        auto tok = getNextToken();
        
        if (!tok || tok->content.empty())
        {
            break;
        }

        if (tok->type == TokType::TagDeclare)
        {
            auto node = parseTagStart(tok->content,tok->isSelfClose);
            if (node)
            {
                root = node;
                
                if (!fileAttr.empty())
                {
                    for (auto pair : fileAttr)
                    {
                        root->setFileAttribute(pair);
                    }
                }
            }
        }
        else if (tok->type == TokType::TagEnd)
        {
            if (root)
            {
                if (tok->isSelfClose)
                {
                    root->isSelfClose = true;
                }
                else
                {
                    if (tok->content != root->tagName)
                    {
                        Util::throwError("the end tag must have the same name to begin tag");
                    }
                }
            }
        }
        else if (tok->type == TokType::FileAttribute)
        {
            if (root)
            {
                Util::throwError("node initilize before parse xml file attribute");
            }
            fileAttr = parseFileAttribute(tok->content);
        }
        
        delete tok;
    }
    
    return root;
}
