#include "UDPSocket.hpp"
#include <cstring>
#include <cerrno>
#include <vector>

UDPSocket::RecvObjType UDPSocket::recvFrom()
{
    sockaddr_storage client;
    bzero(&client, 0);

    Util::byte buffer[recvBuffSize];

    auto naddr = reinterpret_cast<sockaddr *>(&client);
    socklen_t naddrlen = sizeof(sockaddr_storage);

    auto recvBytes = ::recvfrom(sockfd(), buffer, recvBuffSize, 0, naddr, &naddrlen);
    if (recvBytes < 0)
    {
        throwError("recvfrom error " + std::string(gai_strerror(errno)));
    }

    auto bytes = std::vector<Util::byte>(buffer, buffer + recvBytes);
    return std::make_tuple(bytes, client);
}

ssize_t UDPSocket::sendto(void *buffer, size_t len, sockaddr_storage &addr)
{
    if (!buffer || len == 0)
    {
        return 0;
    }

    auto target = reinterpret_cast<sockaddr *>(&addr);
    auto naddrlen = SocketConfig::addressLen(addr);
    if (naddrlen == 0)
    {
        throwError("address len is 0");
    }

    auto sendedBytes = ::sendto(sockfd(), buffer, len, 0, target, naddrlen);
    if (sendedBytes < 0)
    {
        throwError("sendto error " + std::string(gai_strerror(errno)));
    }

    return sendedBytes;
}