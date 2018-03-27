#ifndef Row_hpp
#define Row_hpp

#include <string>

class Row
{
public:
    std::string name;
    std::string type;
public:
    Row();
    Row(std::string sql) : Row() 
    {
        parseStatement(sql);
    }
private:
    void parseStatement(std::string &statement);
};

#endif