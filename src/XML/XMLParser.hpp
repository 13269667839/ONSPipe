#ifndef XMLParser_hpp
#define XMLParser_hpp

#include "XMLLexer.hpp"
#include "XMLDocument.hpp"
#include <stack>
#include <deque>

class XMLParser
{
public:
    XMLParser(std::string _input,InputType _type,bool isHTML = false);
    ~XMLParser();
    
    XMLDocument * xmlTextToDocument();
private:
    std::stack<XMLTok *> tokenStack;
    std::stack<XMLDocument *> elementStack;
    XMLLex *lex;
    bool isHTML;
    std::deque<XMLTok *> *htmlTokQueue;
private:
    XMLTok * getNextToken();
    
    void parse_file_attr();
    
    void parse_tag_declare();
    
    void parse_cdata();
    
    void parse_content();
    
    void parse_tag_end();

    void getAllToken();
    void fixNoneSelfClosedTag();
};

#endif
