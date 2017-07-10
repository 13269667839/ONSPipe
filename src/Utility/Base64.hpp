#ifndef Base64_hpp
#define Base64_hpp

#include <string>

class Base64
{
public:
    static std::string encode(std::string code);
    static std::string decode(std::string code);
};

#endif
