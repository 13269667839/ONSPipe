#ifndef FileSystem_hpp
#define FileSystem_hpp

#include <vector>
#include <string>

class FileSystem 
{
public:
    static std::vector<char> readFile(std::string filePath);

    static bool isDir(std::string path);

    static std::vector<std::string> filesInTheCurrentDirectory(std::string filePath);

    static std::string currentWorkDirectory();
};

#endif