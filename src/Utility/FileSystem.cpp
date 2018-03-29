#include "FileSystem.hpp"
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

bool FileSystem::isDir(std::string path)
{
    struct stat buf;
    return lstat(path.c_str(), &buf) < 0 ? false : S_ISDIR(buf.st_mode);
}

std::vector<char> FileSystem::readFile(std::string filePath)
{
    std::vector<char> chars;

    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open())
    {
        chars = std::vector<char>(file.seekg(0, std::ios::end).tellg(), 0);
        file.seekg(0, std::ios::beg).read(&chars[0], static_cast<std::streamsize>(chars.size()));
        file.close();
    }
    return chars;
}

std::vector<std::string> FileSystem::filesInTheCurrentDirectory(std::string filePath)
{
    std::vector<std::string> files = {};

    if (auto dp = opendir(filePath.c_str()))
    {
        while (auto dirp = readdir(dp))
        {
            files.push_back(dirp->d_name);
        }

        closedir(dp);
    }

    return files;
}

std::string FileSystem::currentWorkDirectory()
{
    auto str = getcwd(nullptr, 0);

    if (!str)
    {
        return std::string();
    }

    auto r_str = std::string(str);
    delete str;
    str = nullptr;

    return r_str;
}