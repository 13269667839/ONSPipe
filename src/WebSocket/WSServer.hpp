#ifndef WSServer_hpp
#define WSServer_hpp

#include "../Socket/Socket.hpp"
#include "../Utility/Util.hpp"

class WSServer 
{
public:
    using Event = std::function<void (const WSServer *server,int fd,std::string msg)>;
public:
    WSServer(int port);
    ~WSServer();
    
    void eventLoop(Event event);
    void sendMsg(std::string msg,int fd) const;
private:
    void setSocket();
    int handShaking();
    
    std::string parseRecvData(Util::byte *recvdata,size_t len);
private:
    Socket *sock;
    int port;
};

#endif 
