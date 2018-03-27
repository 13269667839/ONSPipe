#include "Row.hpp"
#include "../Utility/Util.hpp"
#include <algorithm>

Row::Row()
{
    name = std::string();
    type = std::string();
}

void Row::parseStatement(std::string &statement)
{
    Strings::trim(statement,' ');

    auto idx = statement.find(' ');
    if (idx == std::string::npos)
    {
        return;
    }

    name += statement.substr(0,idx);
    statement = statement.substr(idx + 1);

    auto strs = Strings::split(statement,std::string(" "));
    auto typeSet = std::vector<std::string>{"INTEGER","VARCHAR","TIMESTAMP","BLOB"};

    auto ite = std::find_if(std::begin(strs),std::end(strs),[&typeSet](std::string &str)
    {
        return std::find_if(std::begin(typeSet),std::end(typeSet),[&str](std::string &type)
        {
            return str.find(type) != std::string::npos;
        }) != std::end(typeSet);
    });

    if (ite != std::end(strs))
    {
        type += *ite;
        strs.erase(ite);
    }
}