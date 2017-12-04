#ifndef XMLElement_hpp
#define XMLElement_hpp

#include <vector>
#include <map>
#include <string>
#include <ostream>

struct XMLDocument
{
public:
    std::string tagName;
    std::vector<XMLDocument *> *children;
    std::map<std::string,std::string> *attribute;
    std::map<std::string,std::string> *fileAttribute;
    std::string *content;
    bool isSelfClose;
    bool isCData;
public:
    XMLDocument(std::string _tagName = "");
    ~XMLDocument();
    void setAttribute(std::pair<std::string,std::string> pair);
    void setFileAttribute(std::pair<std::string,std::string> pair);
    void setContent(std::string _content);
    void addChildNode(XMLDocument *obj);
    
    std::string prettyPrint();
private:
    std::string fileAttrPrint();
    std::string nodePrint(int &tabCount);
    std::string tabPrint(int len);
};

std::ostream & operator << (std::ostream &os,XMLDocument *document);

std::ostream & operator << (std::ostream &os,XMLDocument &document);

#endif
