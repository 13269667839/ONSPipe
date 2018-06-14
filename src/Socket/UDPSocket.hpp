#ifndef UDPSocket_hpp
#define UDPSocket_hpp

#include "BasicSocket.hpp"
#include "../Utility/UtilConstant.hpp"

class UDPSocket : public BasicSocket 
{
public:
    UDPSocket(int domain,AddressFamily family) : BasicSocket(domain,SocketType::UDP,family) 
    {
        if (sockfd() == -1)
        {
            throwError("invalid socket fd");
        }
    }

    using RecvObjType = std::tuple<std::vector<Util::byte>, sockaddr_storage>;
    RecvObjType recvFrom();

    ssize_t sendto(void *buffer, size_t len, sockaddr_storage &addr);
};

#endif