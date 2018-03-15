#include "XMLParser.hpp"
#include "../Utility/Util.hpp"
#include <algorithm>

XMLParser::XMLParser(std::string _input,InputType _type,bool isHTML)
{
    this->isHTML = isHTML;
    htmlTokQueue = nullptr;
    
    lex = new XMLLex(_input,_type);

    tokenStack = std::stack<XMLTok *>();
    elementStack = std::stack<XMLDocument *>();
}

XMLParser::~XMLParser()
{
    if (lex)
    {
        delete lex;
        lex = nullptr;
    }

    if (htmlTokQueue)
    {
        htmlTokQueue->clear();
        delete htmlTokQueue;
        htmlTokQueue = nullptr;
    }
}

XMLTok * XMLParser::getNextToken()
{
    XMLTok *tok = nullptr;
    if (isHTML)
    {
        if (htmlTokQueue && !htmlTokQueue->empty())
        {
            tok = htmlTokQueue->at(0);
            htmlTokQueue->pop_front();
        }
    }
    else
    {
        if (lex)
        {
            tok = lex->getNextTok();
        }
    }
    return tok;
}

void XMLParser::getAllToken()
{
    if (!lex) 
    {
        return;
    }

    htmlTokQueue = new std::deque<XMLTok *>();

    while (auto tok = lex->getNextTok())
    {
        if (tok && htmlTokQueue)
        {
            htmlTokQueue->push_back(tok);
        }
    }
}

void XMLParser::fixNoneSelfClosedTag()
{
    auto end = htmlTokQueue->end();
    for (auto ite = htmlTokQueue->begin();ite != end;++ite)
    {
        auto token = *ite;
        if (token->type == TokType::TagDeclare && !token->isSelfClose)
        {
            auto isHave = std::find_if(ite + 1,end,[&token](XMLTok *tok)
            {
                return tok->type == TokType::TagEnd && Strings::isPrefix(token->content,tok->content);
            }) != end;

            if (!isHave)
            {
                token->isSelfClose = true;
            }
        }
    }
}

XMLDocument * XMLParser::xmlTextToDocument()
{
    if (isHTML)
    {
        getAllToken();
        fixNoneSelfClosedTag();
    }

    while (auto tok = getNextToken())
    {
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

    auto name = tok->tagName();
    if (name.empty())
    {
        throwError("empty tag name");
    }
    auto element = new XMLDocument(name,isHTML);
    elementStack.push(element);

    auto attrDic = tok->attribute();
    if (!attrDic.empty())
    {
        for (auto pair : attrDic)
        {
            element->setAttribute(pair.first, pair.second);
        }
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
                    element->setFileAttribute(buf.first,buf.second);
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
                    element->setFileAttribute(buf.first,buf.second);
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
