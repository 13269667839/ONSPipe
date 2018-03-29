#ifndef XMLParser_hpp
#define XMLParser_hpp

#include "XMLLexer.hpp"
#include "XMLDocument.hpp"
#include <stack>
#include <deque>
#include <memory>

class XMLParser
{
public:
    XMLParser(std::string _input,InputType _type,bool isHTML = false);
    ~XMLParser();
    
    std::unique_ptr<XMLDocument> xmlTextToDocument();
private:
    std::stack<XMLTok *> tokenStack;
    std::stack<XMLDocument *> elementStack;
    XMLLex *lex;
    bool isHTML;
    std::deque<XMLTok *> *htmlTokQueue;
private:
    XMLTok * getNextToken();
    
    void parse_tag_declare();
    
    void parse_cdata();
    
    void parse_content();
    
    void parse_tag_end();

    void getAllToken();
    bool fixNoneSelfClosedTag(std::string name);
};

#endif
