#include "XMLParser.hpp"
#include <algorithm>
#include "../Utility/Strings.hpp"

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

bool XMLParser::fixNoneSelfClosedTag(std::string name)
{
    auto end = htmlTokQueue->end();
    return std::find_if(htmlTokQueue->begin(), end, [&name](XMLTok *tok) 
    {
        return tok->type == TokType::TagEnd && Strings::isPrefix(name, tok->content);
    }) == end;
}

std::unique_ptr<XMLDocument> XMLParser::xmlTextToDocument()
{
    if (isHTML)
    {
        getAllToken();
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

    auto head = elementStack.empty() ? nullptr : (elementStack.size() == 1 ? elementStack.top() : nullptr);
    return std::unique_ptr<XMLDocument>(head);
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

    if (isHTML && !tok->isSelfClose)
    {
        tok->isSelfClose = fixNoneSelfClosedTag(name);
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

    if (!tokenStack.empty() && tokenStack.top()->type == TokType::FileAttribute)
    {
        auto fileAttrTok = tokenStack.top();
        tokenStack.pop();

        auto elementTop = elementStack.top();

        auto fileAttr = fileAttrTok->fileAttribute();
        for (auto pair : fileAttr)
        {
            elementTop->setFileAttribute(pair.first, pair.second);
        }

        delete fileAttrTok;
        fileAttrTok = nullptr;
    }
}
