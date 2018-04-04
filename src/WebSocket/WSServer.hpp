#ifndef WSServer_hpp
#define WSServer_hpp

#include "../Socket/Socket.hpp"
#include "../Utility/Util.hpp"

struct WSClientMsg
{
  public:
    std::string msg;
    std::string type;
    bool connect;

    WSClientMsg() : msg(std::string()), type(std::string()), connect(true) {}
};

class WSServer
{
  public:
    WSServer(int port);
    ~WSServer();

  public:
    void sendMsg(std::string msg, int fd) const;
    void loop();

  public:
    std::function<void(int)> start;
    std::function<void(int)> end;

    std::function<void(int, std::string)> error;

    std::function<void(int, WSClientMsg &)> receive;

  private:
    void setSocket();

    WSClientMsg parseRecvData(std::vector<Util::byte> bytes);

    int handShaking();

  private:
    Socket *sock;
    int port;
};

#endif
