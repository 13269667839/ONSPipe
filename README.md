# ONSPipe
a self-customed toy

# config
* paltform:macOS,Linux(Ubuntu)
* compiler:clang++,llvm-g++,g++
* cpp core lib version:std >= c++11

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
