#ifndef SQLite_hpp
#define SQLite_hpp

#include <functional>
#include <sqlite3.h>
#include "ResultSet.hpp"

class SQLite
{
public:
    SQLite(std::string _path);
    ~SQLite();
    void execSQL(std::string sql,std::function<void(ResultSet *set,char *errMsg)> _callback);
    std::vector<std::string> allTablesName();
private:
    sqlite3 *db;
    
    void closeDB();
};

#endif 
