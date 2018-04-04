#ifndef Crypto_hpp
#define Crypto_hpp

#include <string>
#include "UtilConstant.hpp"

namespace Crypto
{
extern std::string b64encode(std::string buffer, bool newline = false);

extern std::string b64decode(std::string buffer, bool newline = false);

extern std::basic_string<Util::byte> sha1encode(Util::byte *src, size_t len);
};

#endif