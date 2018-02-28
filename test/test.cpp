#include <iostream>
#include "../src/HTTP/HTTPServer.hpp"
#include "../src/HTTP/HTTPClient.hpp"
#include "../src/XML/XMLParser.hpp"
#include "../src/JSON/JSONParser.hpp"
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

int main(int argc, const char *argv[])
{
    json();
    return 0;
}
