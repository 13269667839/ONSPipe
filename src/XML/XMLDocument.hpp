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
    bool isHTML;
public:
    XMLDocument(std::string _tagName,bool _isHTML);
    ~XMLDocument();
    void setAttribute(std::string key,std::string value);
    void setFileAttribute(std::string key,std::string value);
    void setContent(std::string _content);
    void addChildNode(XMLDocument *obj);

    XMLDocument * getElementById(std::string id);
    std::vector<XMLDocument *> getElementsByTagName(std::string _tagName);
    std::vector<XMLDocument *> getElementsByClassName(std::string className);
    
    std::string prettyPrint();
private:
    std::string fileAttrPrint();
    std::string nodePrint(int &tabCount);
    std::string tabPrint(int len);
};

std::ostream & operator << (std::ostream &os,XMLDocument *document);

std::ostream & operator << (std::ostream &os,XMLDocument &document);

#endif
