#ifndef SQLite_hpp
#define SQLite_hpp

#include <sqlite3.h>
#include <functional>
#include <map>
#include <vector>
#include <string>

using ResultSet = std::vector<std::map<std::string,std::string>>;
using SQLiteCallback = std::function<void(const ResultSet set,char *errMsg)>;

class SQLite
{
public:
    SQLite(std::string _path);
    ~SQLite();
public:
    void execSQL(std::string sql,SQLiteCallback _callback);
    std::vector<std::string> allTablesName();
private:
    sqlite3 *db;
    
    void closeDB();
};

#endif 
