#ifndef Crypto_hpp
#define Crypto_hpp

#include <string>

namespace Crypto
{
extern std::string b64encode(std::string buffer, bool newline = false);

extern std::string b64decode(std::string buffer, bool newline = false);
};

#endif