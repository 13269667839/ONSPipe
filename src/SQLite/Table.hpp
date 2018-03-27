#ifndef Table_hpp
#define Table_hpp

#include <string>
#include <map>
#include <set>
#include "Row.hpp"

class Table
{
  public:
    std::string name;
    std::set<std::string> *attribute;
    std::map<std::string, Row> *rows;

  public:
    Table();
    ~Table();

    Table(std::string sql) : Table()
    {
        parseSQL(sql);
    }
public:
    void setAttribute(std::string attr);
    void setRow(Row &row);
private:
    void parseSQL(std::string sql);
    void parseDeclare(std::string &declare);
    void parseStatement(std::string &statement);
};

#endif