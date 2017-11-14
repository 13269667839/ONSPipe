#ifndef SQLite_hpp
#define SQLite_hpp

#include <sqlite3.h>
#include <functional>
#include <map>
#include <vector>
#include <string>

using ResultSet = std::vector<std::map<std::wstring,std::wstring>>;
using SQLiteCallback = std::function<void(const ResultSet set,char *errMsg)>;

class SQLite
{
public:
    SQLite(std::string _path);
    ~SQLite();
    void execSQL(std::string sql,SQLiteCallback _callback);
    std::vector<std::wstring> allTablesName();
private:
    sqlite3 *db;
    
    void closeDB();
};

#endif 
