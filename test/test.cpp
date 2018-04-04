#include <iostream>
#include "../src/HTTP/HTTPServer.hpp"
#include "../src/HTTP/HTTPClient.hpp"
using namespace std;

void server()
{
    auto server = HTTPServer(8080);
    server.runAndLoop([](HTTPRequest &request, HTTPResponse &response) {
        cout << request << endl;

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
        cout << *response << endl;
    }
}

int main(int argc, const char *argv[])
{
    return 0;
}
