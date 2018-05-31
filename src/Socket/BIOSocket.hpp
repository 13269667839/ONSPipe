#ifndef BIOSocket_hpp
#define BIOSocket_hpp

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <vector>
#include <string>
#include "../Utility/UtilConstant.hpp"

class BIOSocket 
{
public:
    BIOSocket(bool _isViaSSL = false);
    ~BIOSocket();

    bool connect(std::string host,std::string port);
    ssize_t send(void *buffer,size_t len);
    std::vector<Util::byte> receive(int recvBufSize = 512);
    bool close();
private:
    SSL *ssl;
    SSL_CTX *sslCtx;
    BIO *socketBIO;

    bool isViaSSL;
};

#endif