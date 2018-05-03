# ONSPipe
a self-customed toy

# config
* paltform:macOS,Linux(Ubuntu)
* compiler:clang++,llvm-g++,g++
* cpp core lib version: c++14

# test case

## json
```
#include <JSONParser.hpp>
auto parser = JSONParser(InputType::Text,text);
//auto parser = JSONParser(InputType::File,filePath);
auto obj = parser.token2Object();
if (obj)
{
    cout<<*obj<<endl;
}
```

## xml
```
#include <XMLParser.hpp>
auto parser = XMLParser(text,InputType::Text);
//auto parser = XMLParser(filePath,InputType::File);
auto document = parser.xmlTextToDocument();
if (document)
{
    cout<<*document<<endl;
}
```

## server
```
#include <HTTPServer.hpp>
auto server = HTTPServer(8080);
server.loop([](HTTPRequest &request, HTTPResponse &response) 
{
    cout << request << endl;

    response.setResponseLine("HTTP/1.1", 200, "OK");
    response.body = (unsigned char *)"Hello World!\n";
    response.addResponseHead("Content-Type", "text/plain");
    response.addResponseHead("Content-Length", to_string(response.body.size()));
});
```
