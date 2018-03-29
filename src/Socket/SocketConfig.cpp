#include "SocketConfig.hpp"
#include <cstring>
#include <arpa/inet.h>

addrinfo * SocketConfig::getAddressInfo(std::string address, std::string port, AddressFamily family, SocketType socktype)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    if (family == AddressFamily::Default)
    {
        hints.ai_family = AF_UNSPEC;
    }
    else if (family == AddressFamily::IPV4)
    {
        hints.ai_family = AF_INET;
    }
    else if (family == AddressFamily::IPV6)
    {
        hints.ai_family = AF_INET6;
    }

    if (socktype == SocketType::TCP)
    {
        hints.ai_socktype = SOCK_STREAM;
    }
    else if (socktype == SocketType::UDP)
    {
        hints.ai_socktype = SOCK_DGRAM;
    }

    const char *addr = nullptr;
    if (address.empty())
    {
        hints.ai_flags = AI_PASSIVE; //127.0.0.1
    }
    else
    {
        addr = address.c_str();
    }

    addrinfo *allAddrinfo = nullptr;

    /**
     *  DNS or server name query
     *  @param1 ip or domain name
     *  @param2 serve name or port name
     *  @param3 addr info that you customized
     *  @param4 liked list returns
     */
    auto res = getaddrinfo(addr, port.c_str(), &hints, &allAddrinfo);
    if (res != 0)
    {
        throwError("getaddrinfo error : " + std::string(gai_strerror(res)));
    }

    return allAddrinfo;
}

std::string SocketConfig::netAddressToHostAddress(sockaddr addr)
{
    if (addr.sa_family == AF_INET)
    {
        //ipv4
        char src[INET_ADDRSTRLEN];
        inet_ntop(addr.sa_family, &(((sockaddr_in *)&addr)->sin_addr), src, sizeof(src));
        return std::string(src);
    }
    else if (addr.sa_family == AF_INET6)
    {
        //ipv6
        char src[INET6_ADDRSTRLEN];
        inet_ntop(addr.sa_family, &(((sockaddr_in6 *)&addr)->sin6_addr), src, sizeof(src));
        return std::string(src);
    }
    return std::string();
}

std::string SocketConfig::byteOrder()
{
    union {
        short value;
        char bytes[sizeof(short)];
    } buf;

    buf.value = 0x0102;

    if (buf.bytes[0] == 1 && buf.bytes[1] == 2)
    {
        return "big endian";
    }
    else if (buf.bytes[0] == 2 && buf.bytes[1] == 1)
    {
        return "little endian";
    }

    return "unknown";
}

bool SocketConfig::isIPV4Address(const std::string &address)
{
    sockaddr_in sa;
    return inet_pton(AF_INET, address.c_str(), &sa.sin_addr) == 1;
}

bool SocketConfig::isIPV6Address(const std::string &address)
{
    sockaddr_in6 sa;
    return inet_pton(AF_INET6, address.c_str(), &sa.sin6_addr) == 1;
}