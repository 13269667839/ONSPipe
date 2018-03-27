#include "Table.hpp"
#include "../Utility/Util.hpp"

Table::Table()
{
    name = std::string();
    attribute = nullptr;
    rows = nullptr;
}

Table::~Table()
{
    if (attribute)
    {
        attribute->clear();
        delete attribute;
        attribute = nullptr;
    }

    if (rows)
    {
        rows->clear();
        delete rows;
        rows = nullptr;
    }
}

void Table::setAttribute(std::string attr)
{
    if (attr.empty())
    {
        return;
    }

    if (!attribute)
    {
        attribute = new std::set<std::string>();
    }

    attribute->insert(attr);
}

void Table::setRow(Row &row)
{
    if (row.name.empty())
    {
        return;
    }

    if (!rows)
    {
        rows = new std::map<std::string, Row>();
    }

    rows->insert(std::make_pair(row.name, row));
}

void Table::parseDeclare(std::string &declare)
{
    Strings::trim(declare,' ');
    
    auto idx = declare.find("IF NOT EXISTS");
    if (idx != std::string::npos)
    {
        setAttribute("IF NOT EXISTS");
        declare = declare.replace(declare.begin() + idx,declare.begin() + idx + 13,"");
    }

    idx = declare.find("CREATE TABLE ");
    if (idx != std::string::npos)
    {
        name = declare.substr(13);
    }
}

void Table::parseStatement(std::string &statement)
{
    Strings::trim(statement, ' ');
    auto declares = Strings::split(statement, std::string(","));
    for (auto declare : declares)
    {
        auto row = Row(declare);
        setRow(row);
    }
}

void Table::parseSQL(std::string sql)
{
    if (sql.empty())
    {
        return;
    }

    auto idx = sql.find("(");
    if (idx == std::string::npos || sql.back() != ')')
    {
        return;
    }

    auto declare = sql.substr(0,idx);
    parseDeclare(declare);

    auto statement = sql.substr(idx + 1,sql.size() - idx - 1 - 1);
    parseStatement(statement);
}