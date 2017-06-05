#ifndef URL_hpp
#define URL_hpp

#include <string>
#include <ostream>

class URL
{
public:
    URL(std::string str);
    
    friend std::ostream & operator << (std::ostream &os,URL url);
public:
    std::string originalURL;
    std::string scheme;
    std::string host;
    int portNumber;
    std::string path;
    std::string query;
private:
    void setInitialParameter();
    std::string setScheme(std::string str);
    std::string setHost(std::string str);
    std::string setPath(std::string str);
    void setQuery(std::string str);
};

#endif
