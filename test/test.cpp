#include <iostream>
#include "../src/HTTP/HTTPServer.hpp"

int main()
{
    auto server = HTTPServer(8080);
    server.runAndLoop([](const HTTPRequest &request,HTTPResponse &response)
    {
        response.version = "HTTP/1.1";
        response.statusCode = 200;
        response.reason = "OK";
        
        response.responseBody = "response from server\n";
        
        response.addResponseHead({"Content-Type","text/plain"});
        response.addResponseHead({"Content-Length","21"});
    });
    return 0;
}
