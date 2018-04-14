#include "SocketConfig.hpp"
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>

addrinfo * SocketConfig::getAddressInfo(std::string address, std::string port, AddressFamily family, SocketType socktype)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    auto _family = SocketConfig::addressFamilyRawValue(family);
    if (_family == -1)
    {
        throwError("error family value");
        return nullptr;
    }
    hints.ai_family = _family;

    auto _socktype = SocketConfig::socketTypeRawValue(socktype);
    if (_socktype == -1)
    {
        throwError("error socket type");
        return nullptr;
    }
    hints.ai_socktype = _socktype;

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

//-1 is error
int SocketConfig::socketTypeRawValue(SocketType &type)
{
    auto res = -1;

    switch (type)
    {
    case SocketType::TCP:
        res = SOCK_STREAM;
        break;
    case SocketType::UDP:
        res = SOCK_DGRAM;
        break;
    default:
        break;
    }

    return res;
}

//-1 is error
int SocketConfig::addressFamilyRawValue(AddressFamily &family)
{
    auto res = -1;

    switch (family)
    {
    case AddressFamily::Default:
        res = AF_UNSPEC;
        break;
    case AddressFamily::IPV4:
        res = AF_INET;
        break;
    case AddressFamily::IPV6:
        res = AF_INET6;
        break;
    default:
        break;
    }

    return res;
}

socklen_t SocketConfig::addressLen(sockaddr_storage &addr)
{
    if (addr.ss_family == AF_INET)
    {
        return sizeof(sockaddr_in);
    }
    else if (addr.ss_family == AF_INET6)
    {
        return sizeof(sockaddr_in6);
    }
    return 0;
}

bool SocketConfig::setNonBlocking(int sockfd)
{
    auto flag = fcntl(sockfd, F_GETFL);
    if (flag == -1)
    {
        return false;
    }

    return fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) != -1;
}