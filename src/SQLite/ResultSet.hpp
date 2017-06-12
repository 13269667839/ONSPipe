#ifndef ResultSet_hpp
#define ResultSet_hpp

#include <map>
#include <vector>
#include <string>

class ResultSet
{
public:
    ResultSet();
    ~ResultSet();
    
    void appendRow(std::map<std::string,std::string> *row);
    std::string htmlTable();
    bool empty();
    
    std::vector<std::map<std::string,std::string> *> *rows;
};

#endif 
