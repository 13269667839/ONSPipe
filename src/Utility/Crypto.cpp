#include "Crypto.hpp"
#include <algorithm>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/bio.h>

std::string Crypto::b64encode(std::string buffer, bool newline)
{
    if (buffer.empty())
    {
        return std::string();
    }

    auto b64 = BIO_new(BIO_f_base64());
    if (!newline)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); //write result in one line
    }

    b64 = BIO_push(b64, BIO_new(BIO_s_mem()));
    BIO_write(b64, buffer.c_str(), buffer.size());
    BIO_flush(b64);

    BUF_MEM *ptr = nullptr;
    BIO_get_mem_ptr(b64, &ptr);
    BIO_set_close(b64, BIO_NOCLOSE);

    BIO_free_all(b64);

    if (!ptr)
    {
        return std::string();
    }

    const size_t len = ptr->length;
    char encodeStr[len];
    std::copy(ptr->data, ptr->data + len, encodeStr);

    delete ptr;
    ptr = nullptr;

    return std::string(encodeStr, len);
}

std::string Crypto::b64decode(std::string buffer, bool newline)
{
    if (buffer.empty())
    {
        return std::string();
    }

    auto b64 = BIO_new(BIO_f_base64());
    if (!newline)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }

    auto length = buffer.size();

    auto bmem = BIO_new_mem_buf(buffer.c_str(), length);
    bmem = BIO_push(b64, bmem);

    char decodeStr[length];
    std::fill(decodeStr, decodeStr + length, '\0');

    BIO_read(bmem, decodeStr, length);
    BIO_free_all(bmem);

    return std::string(decodeStr, strlen(decodeStr));
}