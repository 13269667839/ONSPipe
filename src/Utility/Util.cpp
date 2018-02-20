#include "Util.hpp"
#include <cctype>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/des.h>

std::vector<std::string> Util::split(std::string src, std::vector<std::string> tokens)
{
    std::vector<std::string> arr;
    if (!src.empty() && !tokens.empty())
    {
        if (tokens.size() == 1)
        {
            arr = Util::split(src, *begin(tokens));
        }
        else
        {
            auto capitalMap = std::map<char, std::vector<std::string>>();
            for (auto token : tokens)
            {
                char ch = *begin(token);
                if (ch != '\0')
                {
                    auto ite = capitalMap.find(ch);
                    if (ite == end(capitalMap))
                    {
                        capitalMap[ch] = {token};
                    }
                    else
                    {
                        ite->second.push_back(token);
                    }
                }
            }

            if (!capitalMap.empty())
            {
                std::string::size_type lastIndex = -1;
                for (std::string::size_type i = 0; i < src.size(); ++i)
                {
                    char ch = src[i];

                    auto chain = capitalMap[ch];
                    if (!chain.empty())
                    {
                        std::string tok;
                        for (auto item : chain)
                        {
                            if (i + item.size() - 1 < src.size())
                            {
                                if (item == src.substr(i, item.size()))
                                {
                                    tok = item;
                                    break;
                                }
                            }
                        }

                        if (!tok.empty())
                        {
                            if (lastIndex == -1)
                            {
                                lastIndex = 0;
                            }

                            arr.push_back(src.substr(lastIndex, i - lastIndex));

                            i += tok.size() - 1;
                            lastIndex = i + 1;
                        }
                    }
                }

                if (lastIndex != -1 && lastIndex < src.size())
                {
                    arr.push_back(src.substr(lastIndex));
                }
            }
        }
    }
    return arr;
}

std::vector<char> Util::readFileSync(std::string filePath)
{
    std::vector<char> chars;

    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open())
    {
        chars = std::vector<char>(file.seekg(0, std::ios::end).tellg(), 0);
        file.seekg(0, std::ios::beg).read(&chars[0], static_cast<std::streamsize>(chars.size()));
        file.close();
    }
    return chars;
}

std::string Util::currentWorkDirectory()
{
    auto str = getcwd(nullptr, 0);

    if (!str)
    {
        return std::string();
    }

    auto r_str = std::string(str);
    delete str;
    str = nullptr;

    return r_str;
}

bool Util::isDirectory(std::string path)
{
    struct stat buf;
    return lstat(path.c_str(), &buf) < 0 ? false : S_ISDIR(buf.st_mode);
}

std::vector<std::string> Util::filesInTheCurrentDirectory(std::string filePath)
{
    std::vector<std::string> files = {};

    if (auto dp = opendir(filePath.c_str()))
    {
        while (auto dirp = readdir(dp))
        {
            files.push_back(dirp->d_name);
        }

        closedir(dp);
    }

    return files;
}

std::map<std::string,std::string> Util::Argv2Map(const char * argv[],int len,const std::map<std::string,int> rule)
{
    auto dic = std::map<std::string,std::string>();

    for (int i = 0;i < len;++i)
    {
        auto arg = argv[i];

        if (!arg || strlen(arg) == 0)
        {
            continue;
        }

        auto ite = rule.find(arg);
        if (ite == rule.end())
        {
            continue;
        }

        auto count = ite->second;
        auto content = std::string();

        auto j = i;
        for (;j < i + count && j < len;++j)
        {
            if (!content.empty())
            {
                content += " ";
            }
            content += argv[j];
        }
        i = j;

        dic[arg] = content;
    }

    return dic;
}

std::wstring Util::s2ws(const std::string &s)
{
    auto curLocale = setlocale(LC_ALL, "");
    
    auto _Source = s.c_str();
    auto _Dsize = mbstowcs(nullptr, _Source, 0) + 1;
    auto _Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    auto result = std::wstring(_Dest);
    delete []_Dest;
    
    setlocale(LC_ALL, curLocale);
    return result;
}

unsigned long Util::u_strlen(const char *utf8_str)
{
    unsigned long len = 0;
    
    if (!utf8_str)
    {
        return len;
    }
    
    auto size = strlen(utf8_str);
    if (size == 0)
    {
        return len;
    }
    
    for (auto i = 0;i < size;++i)
    {
        unsigned long tmp = utf8_str[i];
        unsigned int bits[8] = {};
        
        auto idx = 7;
        while (tmp != 0 && idx >= 0)
        {
            bits[idx] = tmp % 2;
            idx--;
            tmp /= 2;
        }
        
        if (bits[0] == 1)
        {
            auto count_one = 1;
            while (bits[count_one] == 1)
            {
                count_one++;
            }
            
            i += (count_one - 1);
        }
        
        len++;
    }
    
    return len;
}

char * Util::base64_encoding(const char *buffer, int length, bool newLine)
{
    BIO *bmem = nullptr;
    BIO *b64 = nullptr;
    BUF_MEM *bptr = nullptr;
    
    b64 = BIO_new(BIO_f_base64());
    if (!newLine) 
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, buffer, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    BIO_set_close(b64, BIO_NOCLOSE);
    
    char *buff = (char *)malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;
    BIO_free_all(b64);
    
    return buff;
}

Util::byte * Util::sha1_encode(Util::byte *src,size_t len)
{
    SHA_CTX c;
    Util::byte *dest = (Util::byte *)malloc((SHA_DIGEST_LENGTH + 1) * sizeof(Util::byte));
    memset(dest, 0, SHA_DIGEST_LENGTH + 1);
    if(!SHA1_Init(&c))
    {
        free(dest);
        return nullptr;
    }
    SHA1_Update(&c, src, len);
    SHA1_Final(dest,&c);
    OPENSSL_cleanse(&c,sizeof(c));
    return dest;
}

std::vector<Util::byte> Util::zlib_compress(Util::byte *bytes,size_t len)
{
    if (!bytes || len == 0) 
    {
        return {};
    }

    Byte dest[102400];
    uLong compressedBufferLen = 102400;

    auto err = compress(dest,&compressedBufferLen,bytes,len);
    if (err != Z_OK)
    {
        throwError("zlib compress error : " + std::to_string(err));
        return {};
    }

    auto compressedBuffer = std::vector<Util::byte>(compressedBufferLen);
    for (decltype(compressedBufferLen) i = 0;i < compressedBufferLen;++i)
    {
        compressedBuffer[i] = dest[i];
    }
    return compressedBuffer;
}

std::vector<Util::byte> Util::zlib_uncompress(Util::byte *bytes,size_t len)
{
    if (!bytes || len == 0) 
    {
        return {};
    }

    Byte dest[102400];
    uLong deslLen = 1024000;
    auto err = uncompress(dest,&deslLen,bytes,len);
    if (err != Z_OK)
    {
        throwError("zlib uncompress error : " + std::to_string(err));
        return {};
    }

    auto uncompressedBuffer = std::vector<Util::byte>(deslLen);
    for (decltype(deslLen) i = 0;i < deslLen;++i)
    {
        uncompressedBuffer[i] = dest[i];
    }
    return uncompressedBuffer;
}

int Util::gzlib_compress(Bytef *data, uLong ndata,Bytef *zdata, uLong *nzdata)
{
    if (!data || ndata == 0 || !zdata || *nzdata == 0)
    {
        return -1;
    }

    z_stream c_stream;
    int err = 0;
 
    if(data && ndata > 0) 
    {
        c_stream.zalloc = nullptr;
        c_stream.zfree = nullptr;
        c_stream.opaque = nullptr;

        //只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
        if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) 
        {
            return -1;
        }

        c_stream.next_in  = data;
        c_stream.avail_in  = ndata;
        c_stream.next_out = zdata;
        c_stream.avail_out  = *nzdata;

        while(c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
        {
            if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK) 
            {
                return -1;
            }
        }

        if(c_stream.avail_in != 0) 
        {
            return c_stream.avail_in;
        }

        for (;;) 
        {
            if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) 
            {
                break;
            }

            if(err != Z_OK) 
            {
                return -1;
            }
        }

        if(deflateEnd(&c_stream) != Z_OK) 
        {
            return -1;
        }

        *nzdata = c_stream.total_out;
        
        return 0;
    }
    return -1;
}

int Util::gzlib_uncompress(Byte *zdata, uLong nzdata,Byte *data, uLong *ndata)
{
    if (!data || ndata == 0 || !zdata || *ndata == 0)
    {
        return -1;
    }

    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static char dummy_head[2] = 
    {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };

    d_stream.zalloc = nullptr;
    d_stream.zfree = nullptr;
    d_stream.opaque = nullptr;
    d_stream.next_in  = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;

    //只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
    if(inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) 
    {
        return -1;
    }

    while(d_stream.total_out < *ndata && d_stream.total_in < nzdata) 
    {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) 
        {
            break;
        }
        
        if(err != Z_OK) 
        {
            if(err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) 
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

    if(inflateEnd(&d_stream) != Z_OK) 
    {
        return -1;
    }

    *ndata = d_stream.total_out;
    
    return 0;
}
