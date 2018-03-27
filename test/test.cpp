#include <iostream>
#include "../src/HTTP/HTTPServer.hpp"
#include "../src/HTTP/HTTPClient.hpp"
#include "../src/XML/XMLParser.hpp"
#include "../src/JSON/JSONParser.hpp"
#include "../src/SQLite/SQLite.hpp"
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

void xml() 
{
    auto text = "<root count=\"6\"><layer id=\"1\">1</layer><layer id=\"2\">2</layer><layer id=\"3\">3</layer><layer id=\"4\">4</layer><layer id=\"5\">5</layer><layer id=\"6\">6</layer></root>";
    auto parser = XMLParser(text,InputType::Text);
    auto document = parser.xmlTextToDocument();
    if (document)
    {
        cout<<document<<endl;
        delete document;
        document = nullptr;
    }
}

void json() 
{
    auto text = "{\"1\":[1,2,3,[4,5,{\"name\":\"Onion Shen\"}]],\"12345\":[1,2,3,4,5,6,7,8,9]}";
    auto parser = JSONParser(InputType::Text,text);
    auto obj = parser.token2Object();
    if (obj)
    {
        cout<<obj<<endl;
        delete obj;
        obj = nullptr;
    }
}

void sqlite() 
{
    auto sql = SQLite(FileSystem::currentWorkDirectory() + "test.db");
    auto sqls = vector<string>
    {
        "create table if not exists test (id int primary key,name char(5),age int);",
        "insert into test values(1,'12345',12345);",
        "insert into test values(2,'23456',23456);",
        "insert into test values(3,'45678',45678);",
        "insert into test values(4,'56789',56789);",
        "select * from test;"
    };
    for (auto s : sqls)
    {
        sql.execSQL(s,[&s](ResultSet set,std::string errMsg)
        {
            if (!errMsg.empty()) 
            {
                cout<<s<<" error : "<<errMsg<<endl;
            }
            else 
            {
                for (auto row : set)
                {
                    for (auto pair : row)
                    {
                        cout<<pair.first<<" : "<<pair.second<<endl;
                    }
                }
            }
        });
    }
}

void udpServer()
{
    auto socket = Socket("", 8888, SocketType::UDP);
    if (!socket.bind())
    {
        socket.close();
        return;
    }

    try
    {
        while (true)
        {
            auto recvs = socket.receiveFrom();
            auto bytes = get<0>(recvs);
            cout << "recv from client : " << string(bytes.begin(), bytes.end()) << endl;

            auto addr = get<1>(recvs);
            char chs[] = "Hello World!\n";
            socket.sendto(chs, sizeof(chs) / sizeof(char), addr);
        }
    }
    catch (logic_error error)
    {
        cout << "error occur : " << error.what() << endl;
        socket.close();
    }

    socket.close();
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
