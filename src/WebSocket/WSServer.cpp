#include "WSServer.hpp"
#include <sys/select.h>
#include <sys/errno.h>

WSServer::WSServer(int port)
{
    this->port = port;
    this->sock = nullptr;
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
    
    auto kvs = Util::split(recvMsg, std::string("\r\n"));
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
        key = Util::split(*ite, std::string(": "))[1];
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

std::string WSServer::parseRecvData(Util::byte *recvdata,size_t len)
{
    if (!recvdata || len == 0)
    {
        return "";
    }
    
    auto data = std::vector<Util::byte>();//数据
    auto mask = std::vector<Util::byte>();//掩码(4位)
    
    //第二个字节的后7位表示数据的长度
    auto code_len = recvdata[1] & 127;
    
    if (code_len == 126)
    {
        //等于126，用后面相邻的2个字节来保存一个64bit位的无符号整数作为数据的长度,mask从第5个字节(4)开始
        for (decltype(len) i = 4;i < len;++i)
        {
            if (i >= 8)
            {
                data.push_back(recvdata[i]);
            }
            else 
            {
                mask.push_back(recvdata[i]);
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
                data.push_back(recvdata[i]);
            }
            else 
            {
                mask.push_back(recvdata[i]);
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
                data.push_back(recvdata[i]);
            }
            else 
            {
                mask.push_back(recvdata[i]);
            }
        }
    }
    
    auto raw_str = std::string();

    for (auto idx = 0;idx < data.size();++idx)
    {
        char code = data[idx] ^ mask[idx % 4];
        raw_str += code;
    }
    
    return raw_str;
}

void WSServer::sendMsg(std::string msg,int fd) const
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
        bytes.push_back(126);//126不到1个字节，不用转
        
        auto num = htons(length);//要转成网络字节序
        bytes.push_back(num & 0xFF);
        bytes.push_back((num & 0xFF00) >> 8);
    }
    else 
    {
        bytes.push_back(127);//127不到1个字节，不用转
        
        auto num = htons(length);//要转成网络字节序
        bytes.push_back(num & 0xFF);
        bytes.push_back((num & 0xFF00) >> 8);
    }
    
    for (auto ch : msg) 
    {
        bytes.push_back(ch);
    }
    
    while (!bytes.empty()) 
    {
        auto len = sock->send(bytes.data(), bytes.size(),fd);
        if (len >= bytes.size())
        {
            break;
        }
        else 
        {
            bytes.erase(bytes.begin(), bytes.begin() + len);
        }
    }
}

void WSServer::eventLoop(Event event)
{
    if (!event) 
    {
        throwError("event callback is null");
    }
        
    if (!sock) 
    {
        setSocket();
    }
    
    auto read_fds = fd_set();
    FD_ZERO(&read_fds);
    
    auto master = fd_set();
    FD_ZERO(&master);
    
    const auto listener = sock->socketfd;
    FD_SET(listener,&master);
    
    auto fd_max = listener;

    while (true) 
    {
        read_fds = master;
        
        if (select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            throwError(std::string(gai_strerror(errno)));
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
                auto fd = handShaking();
                
                if (fd != -1)
                {
                    FD_SET(fd, &master);
                    fd_max = std::max(fd, fd_max);
                }
            }
            else 
            {
                while (true)
                {
                    auto recvBuf = sock->receive(i);

                    if (recvBuf.empty())
                    {
                        FD_CLR(i, &master);
                        continue;
                    }

                    auto u8_str = parseRecvData(recvBuf.data(), recvBuf.size());

                    event(this, i, u8_str);
                }
            }
        }
    }
}
