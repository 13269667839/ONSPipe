#ifndef XMLParser_hpp
#define XMLParser_hpp

#include "XMLLexer.hpp"
#include "XMLDocument.hpp"
#include <stack>

class XMLParser
{
public:
    XMLParser(std::string _input,InputType _type);
    ~XMLParser();
    
    XMLDocument * xmlTextToDocument();
private:
    std::stack<XMLTok *> tokenStack;
    std::stack<XMLDocument *> elementStack;
    XMLLex *lex;
private:
    XMLTok * getNextToken();
    
    void parse_file_attr();
    
    void parse_tag_declare();
    std::string parse_tag_name(std::string &content);
    void parse_tag_attr();
    
    void parse_cdata();
    
    void parse_content();
    
    void parse_tag_end();
};

#endif
