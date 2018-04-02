#ifndef Util_hpp
#define Util_hpp

#include "STLExtern.hpp"
#include "FileSystem.hpp"
#include "Strings.hpp"

#include <exception>

#include <zlib.h>

#define throwError(msg) throw std::logic_error(msg)

// #define DEBUG
    
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

class Util
{
public:
    using byte = unsigned char;

    static std::map<std::string,std::string> Argv2Map(const char * argv[],int len,const std::map<std::string,int> rule);
    
    static char * base64_encoding(const char *buffer, int length, bool newLine);
    static byte * sha1_encode(byte *src,size_t len);

    static std::vector<Util::byte> zlib_compress(Util::byte *bytes,size_t len);
    static std::vector<Util::byte> zlib_uncompress(Util::byte *bytes,size_t len);

    /*
     *  gzlib compress
     *  @params data        原始数据
     *  @params ndata       原始数据长度
     *  @params zdata       压缩后的数据
     *  @params nzdata      压缩后的数据长度
     *  @return 0 == OK
     */
    static int gzlib_compress(Bytef *data, uLong ndata,Bytef *zdata, uLong *nzdata);
    /*
     *  gzlib uncompress
     *  @params zdata       压缩后的数据
     *  @params nzdata      压缩后的数据长度
     *  @params data        原始数据
     *  @params ndata       原始数据长度
     *  @return 0 == OK
     */
    static int gzlib_uncompress(Byte *zdata, uLong nzdata,Byte *data, uLong *ndata);
};

#endif
