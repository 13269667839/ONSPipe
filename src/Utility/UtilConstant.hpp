#ifndef UtilConstant_hpp
#define UtilConstant_hpp

// #define DEBUG

#include <exception>

#define throwError(msg) throw std::logic_error(msg)

enum class InputType : int
{
    File = 0,
    Text
};

enum class HTTPMessageParseState : int
{
    Line = 0,
    Header,
    Body
};

namespace Util
{
    using byte = unsigned char;
};

#endif