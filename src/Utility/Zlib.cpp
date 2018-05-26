#include "Zlib.hpp"
#include <cstring>
#include <string>

std::vector<Util::byte> Zlib::zlib_compress(Util::byte *bytes, size_t len)
{
    if (!bytes || len == 0)
    {
        return {};
    }

    Byte dest[102400];
    uLong compressedBufferLen = 102400;

    auto err = compress(dest, &compressedBufferLen, bytes, len);
    if (err != Z_OK)
    {
        throwError("zlib compress error : " + std::to_string(err));
        return {};
    }

    auto compressedBuffer = std::vector<Util::byte>(compressedBufferLen);
    for (decltype(compressedBufferLen) i = 0; i < compressedBufferLen; ++i)
    {
        compressedBuffer[i] = dest[i];
    }
    return compressedBuffer;
}

std::vector<Util::byte> Zlib::zlib_uncompress(Util::byte *bytes, size_t len)
{
    if (!bytes || len == 0)
    {
        return {};
    }

    Byte dest[102400];
    uLong deslLen = 1024000;
    auto err = uncompress(dest, &deslLen, bytes, len);
    if (err != Z_OK)
    {
        throwError("zlib uncompress error : " + std::to_string(err));
        return {};
    }

    auto uncompressedBuffer = std::vector<Util::byte>(deslLen);
    for (decltype(deslLen) i = 0; i < deslLen; ++i)
    {
        uncompressedBuffer[i] = dest[i];
    }
    return uncompressedBuffer;
}

int Zlib::gzlib_compress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata)
{
    if (!data || ndata == 0 || !zdata || *nzdata == 0)
    {
        return -1;
    }

    z_stream c_stream;
    int err = 0;

    if (data && ndata > 0)
    {
        c_stream.zalloc = nullptr;
        c_stream.zfree = nullptr;
        c_stream.opaque = nullptr;

        //只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
        if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
            return -1;
        }

        c_stream.next_in = data;
        c_stream.avail_in = ndata;
        c_stream.next_out = zdata;
        c_stream.avail_out = *nzdata;

        while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
        {
            if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
            {
                return -1;
            }
        }

        if (c_stream.avail_in != 0)
        {
            return c_stream.avail_in;
        }

        for (;;)
        {
            if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
            {
                break;
            }

            if (err != Z_OK)
            {
                return -1;
            }
        }

        if (deflateEnd(&c_stream) != Z_OK)
        {
            return -1;
        }

        *nzdata = c_stream.total_out;

        return 0;
    }
    return -1;
}

int Zlib::gzlib_uncompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata)
{
    if (!data || ndata == 0 || !zdata || *ndata == 0)
    {
        return -1;
    }

    int err = 0;
    //decompression stream
    z_stream d_stream;
    static char dummy_head[2] =
        {
            0x8 + 0x7 * 0x10,
            (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
        };

    d_stream.zalloc = nullptr;
    d_stream.zfree = nullptr;
    d_stream.opaque = nullptr;
    d_stream.next_in = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;

    //只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
    if (inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK)
    {
        return -1;
    }

    while (d_stream.total_out < *ndata && d_stream.total_in < nzdata)
    {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */

        if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
        {
            break;
        }

        if (err != Z_OK)
        {
            if (err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef *)dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
        }
    }

    if (inflateEnd(&d_stream) != Z_OK)
    {
        return -1;
    }

    *ndata = d_stream.total_out;

    return 0;
}