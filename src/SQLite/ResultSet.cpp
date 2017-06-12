#include "ResultSet.hpp"
#include "../Utility/Utility.hpp"

ResultSet::ResultSet()
{
    rows = nullptr;
}

ResultSet::~ResultSet()
{
    if (rows)
    {
        rows->clear();
        delete rows;
        rows = nullptr;
    }
}

void ResultSet::appendRow(std::map<std::string,std::string> *row)
{
    if (!rows)
    {
        rows = new std::vector<std::map<std::string,std::string> *>();
    }
    rows->push_back(row);
}

std::string ResultSet::htmlTable()
{
    std::string headHTML = std::string();
    std::string bodyHTML = std::string();
    if (rows && !rows->empty())
    {
        auto firstItem = *begin(*rows);
        auto headers = std::vector<std::string>();
        for (auto pair : *firstItem)
        {
            headers.push_back("<th>" + pair.first + "</th>");
        }
        
        if (!headers.empty())
        {
            headHTML = Utility::join(headers,"");
            if (!headHTML.empty())
            {
                headHTML = "<tr>\n" + headHTML + "\n</tr>";
            }
        }
        
        if (!headHTML.empty())
        {
            auto bodys = std::vector<std::string>(rows->size());
            for (auto eachRow : *rows)
            {
                auto row = std::vector<std::string>(eachRow->size());
                for (auto pair : *eachRow)
                {
                    row.push_back("<td>" + pair.second + "</td>");
                }
                if (!row.empty())
                {
                    bodys.push_back("<tr>\n" + Utility::join(row,"") + "\n</tr>");
                }
            }
            if (!bodys.empty())
            {
                bodyHTML = Utility::join(bodys,"");
            }
        }
    }
    return headHTML.empty() && bodyHTML.empty()?"":("<table>\n" + headHTML + "\n" + bodyHTML + "\n</table>");
}

bool ResultSet::empty()
{
    return rows && rows->empty();
}
