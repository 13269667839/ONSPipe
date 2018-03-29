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
