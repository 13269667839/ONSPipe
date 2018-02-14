#ifndef URL_hpp
#define URL_hpp

#include <string>
#include <ostream>
#include <map>

class URL
{
public:
    URL(std::string str);
    
    friend std::ostream & operator << (std::ostream &os,URL url);

    std::map<std::string,std::string> queryDic();

public:
    std::string originalURL;
    std::string scheme;
    std::string host;
    int portNumber;
    std::string path;
    std::string query;
private:
    void setInitialParameter();
    void parseURLStr(std::string urlStr);
    std::string urlEncode(std::string &str);
};

#endif
