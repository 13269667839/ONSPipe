#include "WSServer.hpp"
#include <sys/select.h>
#include <sys/errno.h>
#include <algorithm>

WSServer::WSServer(int port)
{
    this->port = port;
    this->sock = nullptr;
    this->webSocketEvents = std::map<std::string,Event>();
}

WSServer::~WSServer()
{
    if (sock) 
    {
        sock->close();
        delete sock;
        sock = nullptr;
    }
}

void WSServer::setSocket()
{
    if (!sock) 
    {
        sock = new Socket("",port);
        
        int yes = 1;
        sock->setSocketOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        
        if (!(sock->bind() && sock->listen()))
        {
            throwError("some error occur on bind or listen function");
        }
    }
}

WSClientMsg WSServer::parseRecvData(std::vector<Util::byte> bytes)
{
    auto msg = WSClientMsg();

    if (bytes.empty())
    {
        return msg;
    }

    /*
    %x0：   表示一个延续帧。当Opcode为0时，表示本次数据传输采用了数据分片，当前收到的数据帧为其中一个数据分片。
    %x1：   表示这是一个文本帧（frame）
    %x2：   表示这是一个二进制帧（frame）
    %x3-7： 保留的操作代码，用于后续定义的非控制帧。
    %x8：   表示连接断开。
    %x9：   表示这是一个ping操作。
    %xA：   表示这是一个pong操作。
    %xB-F： 保留的操作代码，用于后续定义的控制帧。
    */
    auto opcode = bytes[0] & 15;

    if (opcode == 0x8)
    {
        msg.connect = false;
        return msg;
    }

    if (opcode == 0x1)
    {
        msg.type = "text";
    }
    else if (opcode == 0x2)
    {
        msg.type = "bolb";
    }

    auto len = bytes.size();
    
    auto data = std::vector<Util::byte>();//数据
    auto mask = std::vector<Util::byte>();//掩码(4位)
    
    //第二个字节的后7位表示数据的长度
    auto code_len = bytes[1] & 127;
    
    if (code_len == 126)
    {
        //等于126，用后面相邻的2个字节来保存一个64bit位的无符号整数作为数据的长度,mask从第5个字节(4)开始
        for (decltype(len) i = 4;i < len;++i)
        {
            if (i >= 8)
            {
                data.push_back(bytes[i]);
            }
            else 
            {
                mask.push_back(bytes[i]);
            }
        }
    }
    else if (code_len > 126)
    {
        //大于126，用后面相邻的8个字节来保存一个64bit位的无符号整数作为数据的长度,mask从第11个字节(10)开始
        for (decltype(len) i = 10;i < len;++i)
        {
            if (i >= 14)
            {
                data.push_back(bytes[i]);
            }
            else 
            {
                mask.push_back(bytes[i]);
            }
        }
    }
    else 
    {
        //小于126时playload只有1位，所以mask从第三位(2)开始
        for (decltype(len) i = 2;i < len;++i)
        {
            if (i >= 6)
            {
                data.push_back(bytes[i]);
            }
            else 
            {
                mask.push_back(bytes[i]);
            }
        }
    }

    for (decltype(data.size()) idx = 0;idx < data.size();++idx)
    {
        char code = data[idx] ^ mask[idx % 4];
        msg.msg += code;
    }
    
    return msg;
}

void WSServer::sendMsg(std::string msg, int fd) const
{
    if (!sock)
    {
        throwError("socket is null");
        return;
    }

    if (fd == -1)
    {
        throwError("invalid file descriptor");
        return;
    }

    if (msg.empty())
    {
        return;
    }

    auto bytes = std::vector<Util::byte>();
    bytes.push_back(0x81);

    auto length = msg.size();
    if (length < 126)
    {
        bytes.push_back(length);
    }
    else if (length <= 0xFFFF)
    {
        bytes.push_back(126); //126不到1个字节，不用转

        auto num = htons(length); //要转成网络字节序
        bytes.push_back(num & 0xFF);
        bytes.push_back((num & 0xFF00) >> 8);
    }
    else
    {
        bytes.push_back(127); //127不到1个字节，不用转

        auto num = htons(length); //要转成网络字节序
        bytes.push_back(num & 0xFF);
        bytes.push_back((num & 0xFF00) >> 8);
    }

    bytes.insert(std::end(bytes), std::begin(msg), std::end(msg));
    sock->sendAll(bytes.data(), bytes.size(), false, fd);
}

#pragma mark -- Event
const std::string WSServer::start = "start";
const std::string WSServer::receive = "receive";
const std::string WSServer::end = "end";
const std::string WSServer::error = "error";

void WSServer::loop()
{
    setSocket();

    auto read_fds = fd_set();
    FD_ZERO(&read_fds);

    auto master = fd_set();
    FD_ZERO(&master);

    const auto listener = sock->socketfd;
    if (listener == -1)
    {
        triggerEvent(WSServer::error,-1,"listener fd is invalid");
        return;
    }
    FD_SET(listener, &master);

    auto fd_max = listener;

    while (true) 
    {
        read_fds = master;

        if (select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            triggerEvent(WSServer::error,-1,std::string(gai_strerror(errno)));
            return;
        }

        for (auto i = 0;i <= fd_max;++i)
        {
            if (!FD_ISSET(i,&read_fds))
            {
                continue;
            }

            if (i == listener)
            {
                auto clientfd = handShaking();
                if (clientfd != -1)
                {
                    FD_SET(clientfd, &master);
                    fd_max = std::max(clientfd, fd_max);
                    triggerEvent(WSServer::start,clientfd,"");
                }
                continue;
            }

            while (true)
            {
                try 
                {
                    auto recvBuf = sock->receive(i);
                    if (recvBuf.empty())
                    {
                        triggerEvent(WSServer::end,i,"");
                        FD_CLR(i, &master);
                        break;
                    }
                    else 
                    {
                        auto msg = parseRecvData(recvBuf);
                        if (msg.connect)
                        {
                            triggerEvent(WSServer::receive,i,msg.msg);
                        }
                        else 
                        {
                            triggerEvent(WSServer::end,i,"");
                            FD_CLR(i, &master);
                            break;
                        }
                    }
                }
                catch (std::logic_error err) 
                {
                    triggerEvent(WSServer::error,i,err.what());
                    FD_CLR(i, &master);
                    break;
                }
            }
        }
    }
}

void WSServer::addEventListener(std::string tag,Event event)
{
    if (!tag.empty() && event) 
    {
        webSocketEvents[tag] = event;
    }
}

void WSServer::triggerEvent(std::string tag,int fd,std::string msg)
{
    auto iter = webSocketEvents.find(tag);
    if (iter != std::end(webSocketEvents))
    {
        iter->second(fd,msg);
    }
}

#pragma mark -- Hand Shaking
int WSServer::handShaking()
{
    if (!sock)
    {
        throwError("socket is null");
    }
    
    auto fd = sock->accept();
    if (fd == -1)
    {
        return -1;
    }

    auto recvMsg = std::string();
    while (1) 
    {       
        auto recvbuf = sock->receive(fd);
        
        if (recvbuf.empty()) 
        {
            break;
        }
        
        recvMsg += (char *)recvbuf.data();
        
        if (recvMsg.rfind("\r\n\r\n") != std::string::npos)
        {
            break;
        }
    }
    
    if (recvMsg.empty())
    {
        return -1;
    }
    
    auto kvs = Strings::split(recvMsg, std::string("\r\n"));
    if (kvs.empty())
    {
        return -1;
    }
    
    auto key = std::string();
    auto ite = std::find_if(kvs.rbegin(), kvs.rend(), [](auto item)
    {
        return item.find("Sec-WebSocket-Key: ") != std::string::npos;
    });
    if (ite != kvs.rend())
    {
        key = Strings::split(*ite, std::string(": "))[1];
    }
    
    if (key.empty())
    {
        return -1;
    }
    
    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    
    auto sha1_str = Util::sha1_encode((Util::byte *)(const_cast<char *>(key.c_str())), key.size());
    if (!sha1_str) 
    {
        return -1;
    }
    
    auto u8_str = (char *)sha1_str;
    auto b64_str = Util::base64_encoding(u8_str, (int)strlen(u8_str), false);
    
    delete sha1_str;
    sha1_str = nullptr;
    
    if (!b64_str) 
    {
        return -1;
    }
    
    key = b64_str;
    
    delete b64_str;
    b64_str = nullptr;
    
    auto line = std::string("HTTP/1.1 101 Switching Protocols\r\n");
    auto header = std::string("Upgrade: websocket\r\n") + "Connection: Upgrade\r\n" + "Sec-WebSocket-Accept: " + key + "\r\n\r\n";
    auto send_msg = line + header;
    sock->sendAll(const_cast<char *>(send_msg.c_str()),send_msg.size(),false,fd);
    
    return fd;
}
