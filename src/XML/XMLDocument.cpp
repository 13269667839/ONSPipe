#include "XMLDocument.hpp"

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

#pragma mark -- pretty print
std::ostream & operator << (std::ostream &os,XMLDocument &document)
{
    os<<document.prettyPrint();
    return os;
}

std::ostream & operator << (std::ostream &os,XMLDocument *document)
{
    if (document)
    {
        os<<document;
    }
    return os;
}

std::string XMLDocument::prettyPrint()
{
    auto out = std::string();
    
    out += fileAttrPrint();
    
    int count = 0;
    out += nodePrint(count);
    
    return out;
}

std::string XMLDocument::tabPrint(int len)
{
    auto res = std::string();
    
    if (len > 0) 
    {
        const auto space = std::string("\t");
        for (auto i = 0;i < len;++i)
        {
            res += space;
        }
    }
    
    return res;
}

std::string XMLDocument::nodePrint(int &tabCount)
{
    auto out = std::string();
    
    out += tabPrint(tabCount);
    
    out += "<" + tagName;
    
    if (attribute && !attribute->empty())
    {
        for (auto ite = attribute->rbegin();ite != attribute->rend();++ite)
        {
            out += " " + ite->first + "=" + "\"" + ite->second + "\"";
        }
    }
    
    if (isSelfClose)
    {
        out += "/>\n";
        return out;
    }
    
    out += ">\n";
    
    if (content)
    {
        out += tabPrint(tabCount + 1);
        
        if (isCData)
        {
            out += "<![CDATA[";
        }
        
        out += *content;
        
        if (isCData)
        {
            out += "]]>";
        }
        
        out += "\n";
    }
    else if (children && !children->empty())
    {
        tabCount++;
        for (auto element : *children)
        {
            out += element->nodePrint(tabCount);
        }
        tabCount--;
    }
    
    out += tabPrint(tabCount) + "</" + tagName + ">\n";
    
    return out;
}

std::string XMLDocument::fileAttrPrint()
{
    auto out = std::string();
    
    if (fileAttribute && !fileAttribute->empty())
    {
        out += "<?xml";
        
        for (auto ite = fileAttribute->rbegin();ite != fileAttribute->rend();++ite)
        {
            out += " " + ite->first + "=" + "\"" + ite->second + "\"";
        }
        
        out += "?>\n";
    }
    
    return out;
}
