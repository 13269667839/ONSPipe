#include <iostream>
#include "../src/HTTP/HTTPServer.hpp"
#include "../src/HTTP/HTTPClient.hpp"
#include "../src/XML/XMLParser.hpp"
#include "../src/JSON/JSONParser.hpp"
#include "../src/SQLite/SQLite.hpp"
using namespace std;

void server()
{
    auto server = HTTPServer(8080);
    server.runAndLoop([](HTTPRequest &request, HTTPResponse &response) {
        cout<<request<<endl;

        response.setResponseLine("HTTP/1.1", 200, "OK");
        
        response.responseBody = (unsigned char *)"Hello World!\n";

        response.addResponseHead("Content-Type", "text/plain");
        response.addResponseHead("Content-Length", to_string(response.responseBody.size()));
    });
}

void request()
{
    auto client = HTTPClient("https://cn.bing.com/");

    client.setRequestHeader("accept-language", "zh-CN,zh;q=0.9,en;q=0.8");
    client.setRequestHeader("Accept-Encoding", "gzip, deflate, br");
    client.setRequestHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36");
    client.setRequestHeader("upgrade-insecure-requests", "1");
    client.setRequestHeader("accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
    client.setRequestHeader("cache-control", "max-age=0");

    auto response = client.syncRequest();
    if (response)
    {
        cout << response << endl;
        delete response;
        response = nullptr;
    }
}

void xml() 
{
    auto text = "<root><layer>1</layer><layer>2</layer><layer>3</layer><layer>4</layer><layer>5</layer><layer>6</layer></root>";
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
    auto parser = JSONParser(SourceType::Text,text);
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
    auto sql = SQLite(Util::currentWorkDirectory() + "test.db");
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
        sql.execSQL(s,[&s](ResultSet set,char *errMsg)
        {
            if (errMsg) 
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

int main(int argc, const char *argv[])
{
    return 0;
}
