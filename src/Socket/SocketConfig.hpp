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

namespace SocketConfig 
{
    /**
     * DNS or server name query
     * @param   address      ip or domain name
     * @param   port         serve name or port name
     * @param   family       address family 
     * @param   socktype     socket type 
     * @return  address info linked list
    */
    extern addrinfo * getAddressInfo(std::string address,std::string port,AddressFamily family,SocketType socktype);

    ///net address to host address
    extern std::string netAddressToHostAddress(sockaddr addr);

    ///machine byte order
    extern std::string byteOrder();

    extern bool isIPV4Address(const std::string &address);

    extern bool isIPV6Address(const std::string &address);

    //-1 is error
    extern int socketTypeRawValue(SocketType &type);

    //-1 is error
    extern int addressFamilyRawValue(AddressFamily &family);

    extern socklen_t addressLen(sockaddr_storage &addr);

    extern bool setNonBlocking(int sockfd);

    template <typename SockPtrType,typename ByteType>
    void sendAll(SockPtrType socket,ByteType bytes,size_t size)
    {
        if (!socket)
        {
            return;
        }

        size_t sendBytes = 0;
        while (sendBytes < size) 
        {
            auto _sendBytes = socket->send(&bytes[sendBytes],size - sendBytes);
            if (_sendBytes == 0) 
            {
                break;
            }
            sendBytes += _sendBytes;
        }
    }
};

#endif