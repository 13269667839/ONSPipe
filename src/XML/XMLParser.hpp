#ifndef XMLParser_hpp
#define XMLParser_hpp

#include "XMLLexer.hpp"
#include "XMLDocument.hpp"

class XMLParser
{
public:
    XMLParser();
    ~XMLParser();
    
    XMLDocument *xmlTextToDocument();
    
    XMLLex *lex;
private:
    XMLTok * getNextToken();
    
    XMLDocument * parseTagStart(std::string lexStr,bool isSelfClose);
    std::string parseTagName(std::string &lexStr);
    void parseTagAttribute(std::string &attrStr,XMLDocument *root);
    std::vector<std::pair<std::string, std::string>> parseFileAttribute(std::string &lexStr);
};

#endif
