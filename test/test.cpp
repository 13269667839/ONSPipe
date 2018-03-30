#include <iostream>
#include "../src/HTTP/HTTPServer.hpp"
#include "../src/HTTP/HTTPClient.hpp"
#include "../src/WebSocket/WSServer.hpp"
using namespace std;

void server()
{
    auto server = HTTPServer(8080);
    server.runAndLoop([](HTTPRequest &request, HTTPResponse &response) {
        cout<<request<<endl;

        response.setResponseLine("HTTP/1.1", 200, "OK");
        
        response.body = (unsigned char *)"Hello World!\n";

        response.addResponseHead("Content-Type", "text/plain");
        response.addResponseHead("Content-Length", to_string(response.body.size()));
    });
}

void request()
{
    auto client = HTTPClient("https://cn.bing.com/");
    client.setRequestHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36");
    auto response = client.request();
    if (response)
    {
        cout<<*response<<endl;
    }
}

void wsserver() 
{
    auto ws = WSServer(9999);
    ws.addEventListener(WSServer::start,[](int fd,std::string msg)
    {
        cout<<"receive new client, id "<<fd<<endl;
    });
    ws.addEventListener(WSServer::receive,[&ws](int fd,std::string msg)
    {
        cout<<"current client id is "<<fd<<",receive msg is "<<msg<<endl;
        ws.sendMsg(msg,fd);
    });
    ws.addEventListener(WSServer::end,[](int fd,std::string msg)
    {
        cout<<"connection closed, id "<<fd<<endl;
    });
    ws.addEventListener(WSServer::error,[](int fd,std::string msg)
    {
        cout<<"client id "<<fd<<",server error "<<msg<<endl;
    });
    ws.loop();
}

int main(int argc, const char *argv[])
{
    return 0;
}
