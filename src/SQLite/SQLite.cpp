#include "SQLite.hpp"
#include "../Utility/Util.hpp"

SQLite::SQLite(std::string _path)
{
    db = nullptr;
    
    if (!_path.empty())
    {
        if (sqlite3_open(_path.c_str(), &db) != SQLITE_OK)
        {
            auto errMsg = sqlite3_errmsg(db);
            closeDB();
            Util::throwError(errMsg);
        }
    }
}

SQLite::~SQLite()
{
    closeDB();
}

void SQLite::closeDB()
{
    if (db)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

static int sql_callback(void *arg,int count,char **columns,char **rows)
{
    if (!arg)
    {
        return 0;
    }
    
    ResultSet *set = static_cast<ResultSet *>(arg);
    
    if (!set)
    {
        return 0;
    }
    
    auto eachRow = std::map<std::wstring,std::wstring>();
    for (int i = 0;i < count;++i)
    {
        auto row = rows[i];
        if (!row || strlen(row) == 0)
        {
            continue;
        }
        
        auto column = columns[i];
        
        auto key = Util::s2ws(row);
        auto value = column?Util::s2ws(column):L"";
        eachRow[key] = value;
    }
    
    if (!eachRow.empty())
    {
        set->push_back(eachRow);
    }
    
    return 0;
}

void SQLite::execSQL(std::string sql,SQLiteCallback _callback)
{
    if (!sql.empty() && _callback && db)
    {
        auto set = ResultSet();
        char *err = nullptr;
        sqlite3_exec(db, sql.c_str(), sql_callback, &set, &err);
        _callback(set,err);
    }
}

std::vector<std::wstring> SQLite::allTablesName()
{
    auto names = std::vector<std::wstring>();
    if (db)
    {
        execSQL("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name", [&names](const ResultSet set,char *err)
        {
            if (!err && !set.empty())
            {
                for (auto row : set)
                {
                    auto name = row[L"name"];
                    if (!name.empty())
                    {
                        names.push_back(name);
                    }
                }
            }
        });
    }
    return names;
}
