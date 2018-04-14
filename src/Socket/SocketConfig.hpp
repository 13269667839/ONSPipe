#ifndef SocketConfig_hpp
#define SocketConfig_hpp

#include "../Utility/Util.hpp"

#include <netdb.h>
#include <sys/socket.h>

enum class SocketType : int
{
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
};

enum class AddressFamily : int 
{
    Default = AF_UNSPEC,
    IPV4    = AF_INET,
    IPV6    = AF_INET6
};

class SocketConfig 
{
public:
    /**
     * DNS or server name query
     * @param   address      ip or domain name
     * @param   port         serve name or port name
     * @param   family       address family 
     * @param   socktype     socket type 
     * @return  address info linked list
    */
    static addrinfo * getAddressInfo(std::string address,std::string port,AddressFamily family,SocketType socktype);

    ///net address to host address
    static std::string netAddressToHostAddress(sockaddr addr);

    ///machine byte order
    static std::string byteOrder();

    static bool isIPV4Address(const std::string &address);

    static bool isIPV6Address(const std::string &address);

    //-1 is error
    static int socketTypeRawValue(SocketType &type);

    //-1 is error
    static int addressFamilyRawValue(AddressFamily &family);

    static socklen_t addressLen(sockaddr_storage &addr);

    static bool setNonBlocking(int sockfd);
};

#endif