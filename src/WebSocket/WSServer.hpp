#ifndef WSServer_hpp
#define WSServer_hpp

#include "../Socket/Socket.hpp"
#include "../Utility/Util.hpp"

using Event = std::function<void (int fd,std::string msg)>;

struct WSClientMsg 
{
public:
    std::string msg;
    std::string type; 
    bool connect;

    WSClientMsg() :msg(std::string()),type(std::string()),connect(true) {}
};

class WSServer 
{
public:
    WSServer(int port);
    ~WSServer();
public:
    void sendMsg(std::string msg,int fd) const;
    void loop();
    void addEventListener(std::string tag,Event event);
public:
    static const std::string start;
    static const std::string receive;
    static const std::string end;
    static const std::string error;
private:
    void triggerEvent(std::string tag,int fd,std::string msg);
    void setSocket();
    
    WSClientMsg parseRecvData(std::vector<Util::byte> bytes);

    int handShaking();
private:
    Socket *sock;
    int port;
    std::map<std::string,Event> webSocketEvents;
};

#endif 
