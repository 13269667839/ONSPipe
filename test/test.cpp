#include <iostream>
#include "../src/HTTP/HTTPServer.hpp"
#include "../src/HTTP/HTTPClient.hpp"
using namespace std;

void server()
{
    auto server = HTTPServer(8080);
    server.runAndLoop([](HTTPRequest &request, HTTPResponse &response) {
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

int main(int argc, const char *argv[])
{
    return 0;
}
