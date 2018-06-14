#ifndef TCPSocket_hpp
#define TCPSocket_hpp

#include "BasicSocket.hpp"
#include "../Utility/UtilConstant.hpp"
#include <memory>

class TCPSocket : public BasicSocket
{
public:
    TCPSocket(int domain,AddressFamily family) : BasicSocket(domain,SocketType::TCP,family) 
    {
        if (sockfd() == -1)
        {
            throwError("invalid socket fd");
        }
    }

    TCPSocket(int sockfd,sockaddr_storage *addr,SocketType _socktype) : BasicSocket(sockfd,addr,_socktype) {}
public:
    bool listen(int backlog = 10);

    bool connect(std::string address, int port);

    ssize_t send(void *buffer,size_t len);

    std::vector<Util::byte> receive();

    std::shared_ptr<TCPSocket> accept();

    template <typename BufferType>
    void sendAll(BufferType buffer,size_t size)
    {
        size_t sendBytes = 0;
        while (sendBytes < size) 
        {
            auto _sendBytes = send(&buffer[sendBytes],size - sendBytes);
            sendBytes += _sendBytes;
        }
    }
};

#endif