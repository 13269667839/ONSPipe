#ifndef Util_hpp
#define Util_hpp

#include "STLExtern.hpp"
#include "FileSystem.hpp"
#include "Strings.hpp"
#include "UtilConstant.hpp"
#include "Crypto.hpp"

#include <zlib.h>

class Utility
{
public:
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
