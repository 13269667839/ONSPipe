#include "XMLDocument.hpp"
#include <exception>

void XMLDocument::addChildNode(XMLDocument *obj)
{
    if (content)
    {
        throw std::logic_error("ethier children or content");
    }
    
    if (obj)
    {
        if (!children)
        {
            children = new std::vector<XMLDocument *>();
        }
        children->push_back(obj);
    }
}

void XMLDocument::setContent(std::string _content)
{
    if (children)
    {
        throw std::logic_error("ethier children or content");
    }
    
    if (!content)
    {
        content = new std::string();
    }
    content->assign(_content);
}

void XMLDocument::setFileAttribute(std::pair<std::string,std::string> pair)
{
    if (!fileAttribute)
    {
        fileAttribute = new std::map<std::string,std::string>();
    }
    
    if (!pair.first.empty())
    {
        fileAttribute->insert(pair);
    }
}

void XMLDocument::setAttribute(std::pair<std::string,std::string> pair)
{
    if (!attribute)
    {
        attribute = new std::map<std::string,std::string>();
    }
    
    if (!pair.first.empty())
    {
        attribute->insert(pair);
    }
}

XMLDocument::XMLDocument(std::string _tagName)
{
    tagName = _tagName;
    content = nullptr;
    attribute = nullptr;
    children = nullptr;
    fileAttribute = nullptr;
    isSelfClose = false;
    isCData = false;
}

XMLDocument::~XMLDocument()
{
    if (attribute)
    {
        attribute->clear();
        delete attribute;
        attribute = nullptr;
    }
    
    if (fileAttribute)
    {
        fileAttribute->clear();
        delete fileAttribute;
        fileAttribute = nullptr;
    }
    
    if (content)
    {
        content->clear();
        delete content;
        content = nullptr;
    }
    
    if (children)
    {
        for (auto ite : *children)
        {
            if (ite)
            {
                delete ite;
                ite = nullptr;
            }
        }
        std::vector<XMLDocument *>().swap(*children);
        delete children;
        children = nullptr;
    }
}
