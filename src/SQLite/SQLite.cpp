#include "SQLite.hpp"
#include "../Utility/Util.hpp"
#include <cstring>

SQLite::SQLite(std::string _path)
{
    db = nullptr;
    tables = nullptr;
    
    if (!_path.empty())
    {
        if (sqlite3_open(_path.c_str(), &db) != SQLITE_OK)
        {
            auto errMsg = sqlite3_errmsg(db);
            closeDB();
            throwError(errMsg);
        }
        parseTableInfo();
    }
}

SQLite::~SQLite()
{
    closeDB();
    if (tables)
    {
        tables->clear();
        delete tables;
        tables = nullptr;
    }
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
    
    auto eachRow = std::map<std::string,std::string>();
    for (int i = 0;i < count;++i)
    {
        auto row = rows[i];
        if (!row || strlen(row) == 0)
        {
            continue;
        }
        
        auto column = columns[i];
        
        auto key = row;
        auto value = column?column:"";
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
        auto errMsg = std::string();
        if (err) 
        {
            errMsg += err;
            delete err;
            err = nullptr;
        } 
        _callback(set,errMsg);
    }
}

void SQLite::parseTableInfo()
{
    if (!db) 
    {
        return;
    }

    auto self = this;
    execSQL("select sql from sqlite_master where type='table';",[&self](const ResultSet set,std::string err) 
    {
        if (!err.empty() && set.empty())
        {
            return;
        }

        for (auto row : set)
        {
            auto sql = row["sql"];
            if (sql.empty())
            {
                continue;
            }

            auto table = new Table(sql);
            self->addTable(table);
        }
    });
}

void SQLite::addTable(Table *table)
{
    if (!table || table->name.empty())
    {
        return;
    }

    if (!tables)
    {
        tables = new std::map<std::string,Table *>();
    }

    tables->insert(std::make_pair(table->name,table));
}
