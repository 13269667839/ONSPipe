#ifndef SQLite_hpp
#define SQLite_hpp

#include <sqlite3.h>
#include <functional>
#include <map>
#include <vector>
#include "Table.hpp"

using ResultSet = std::vector<std::map<std::string,std::string>>;
using SQLiteCallback = std::function<void(ResultSet,std::string)>;

class SQLite
{
public:
    SQLite(std::string _path);
    ~SQLite();
public:
    void execSQL(std::string sql,SQLiteCallback _callback);
    void parseTableInfo();
private:
    sqlite3 *db;
    std::map<std::string,Table *> *tables;
    
    void closeDB();
    void addTable(Table *table);
};

#endif 
