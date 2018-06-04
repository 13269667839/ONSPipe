#include "BIOSocket.hpp"
#include <openssl/err.h>
#include <algorithm>

BIOSocket::BIOSocket(bool _isViaSSL)
{
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    SSL_library_init();

    ssl = nullptr;
    sslCtx = nullptr;
    socketBIO = nullptr;
    isViaSSL = _isViaSSL;
}

BIOSocket::~BIOSocket()
{
    if (ssl)
    {
        close();

        SSL_free(ssl);
        ssl = nullptr;
    }

    if (sslCtx)
    {
        SSL_CTX_free(sslCtx);
        sslCtx = nullptr;
    }

    if (socketBIO)
    {
        if (!isViaSSL)
        {
            BIO_free_all(socketBIO);
        }
        socketBIO = nullptr;
    }

    ERR_free_strings();
}

bool BIOSocket::close()
{
    if (!isViaSSL || !ssl)
    {
        return true;
    }

    //shut down ssl (1:success,0:Unidirectional closed,-1:error)
    auto code = SSL_shutdown(ssl);
    if (code == -1)
    {
        auto err = ERR_reason_error_string(ERR_get_error());
        if (err)
        {
            auto reason = "SSL_shutdown() error : " + std::string(err);
            throwError(reason);
        }
        return false;
    }

    return true;
}

bool BIOSocket::connect(std::string host, std::string port)
{
    auto host_port = host + ":" + port;

    if (isViaSSL)
    {
        sslCtx = SSL_CTX_new(SSLv23_client_method());
        socketBIO = BIO_new_ssl_connect(sslCtx);
        if (!socketBIO)
        {
            auto error = ERR_reason_error_string(ERR_get_error());
            if (error)
            {
                auto reason = "BIO_new_ssl_connect() error = " + std::string(error);
                throwError(reason);
            }
            return false;
        }

        BIO_get_ssl(socketBIO, &ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        BIO_set_conn_hostname(socketBIO, host_port.c_str());
    }
    else
    {
        socketBIO = BIO_new_connect(host_port.c_str());
        if (!socketBIO)
        {
            auto error = ERR_reason_error_string(ERR_get_error());
            if (error)
            {
                auto reason = "BIO_new_connect() error = " + std::string(error);
                throwError(reason);
            }
            return false;
        }
    }

    if (BIO_do_connect(socketBIO) <= 0)
    {
        auto error = ERR_reason_error_string(ERR_get_error());
        if (error)
        {
            auto reason = "BIO_do_connect() error = " + std::string(error);
            throwError(reason);
        }
        return false;
    }

    return true;
}

ssize_t BIOSocket::send(void *buffer, size_t len)
{
    if (!buffer || len <= 0)
    {
        return 0;
    }

    if (!socketBIO)
    {
        throwError("socket bio is null");
        return -1;
    }

    return BIO_write(socketBIO, buffer, len);
}

std::vector<Util::byte> BIOSocket::receive(int recvBufSize)
{
    if (!socketBIO)
    {
        throwError("socket bio is null");
        return {};
    }

    Util::byte buffer[recvBufSize];
    std::fill(buffer, buffer + recvBufSize, '\0');

    auto bytes = BIO_read(socketBIO, buffer, recvBufSize);

    return std::vector<Util::byte>(buffer, buffer + bytes);
}