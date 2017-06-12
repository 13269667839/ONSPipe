#include "SQLite.hpp"
#include "../Utility/Utility.hpp"

SQLite::SQLite(std::string _path)
{
    db = nullptr;
    
    if (!_path.empty())
    {
        if (sqlite3_open(_path.c_str(), &db) != SQLITE_OK)
        {
            auto errMsg = sqlite3_errmsg(db);
            closeDB();
            Utility::throwError(errMsg);
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
    if (arg)
    {
        ResultSet *set = static_cast<ResultSet *>(arg);
        
        auto eachRow = new std::map<std::string,std::string>();
        for (int i = 0;i < count;++i)
        {
            char *row = rows[i];
            char *column = columns[i];
            eachRow->insert({row?row:"",column?column:""});
        }
        
        if (!eachRow->empty())
        {
            set->appendRow(eachRow);
        }
    }
    return 0;
}

void SQLite::execSQL(std::string sql,std::function<void(ResultSet *set,char *errMsg)> _callback)
{
    if (!sql.empty() && _callback && db)
    {
        ResultSet *set = new ResultSet();
        char *err = nullptr;
        sqlite3_exec(db, sql.c_str(), sql_callback, set, &err);
        _callback(set,err);
        delete set;
    }
}

std::vector<std::string> SQLite::allTablesName()
{
    auto names = std::vector<std::string>();
    if (db)
    {
        execSQL("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name", [&names](ResultSet *set,char *err)
        {
            if (!err && set && !set->empty())
            {
                for (auto row : *set->rows)
                {
                    auto name = std::string(row->at("name"));
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
