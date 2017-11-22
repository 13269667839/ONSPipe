#include "Util.hpp"
#include <cctype>
#include <exception>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

std::vector<std::string> Util::split(std::string src, std::vector<std::string> tokens)
{
    std::vector<std::string> arr;
    if (!src.empty() && !tokens.empty())
    {
        if (tokens.size() == 1)
        {
            arr = Util::split(src, *begin(tokens));
        }
        else
        {
            auto capitalMap = std::map<char, std::vector<std::string>>();
            for (auto token : tokens)
            {
                char ch = *begin(token);
                if (ch != '\0')
                {
                    auto ite = capitalMap.find(ch);
                    if (ite == end(capitalMap))
                    {
                        capitalMap[ch] = {token};
                    }
                    else
                    {
                        ite->second.push_back(token);
                    }
                }
            }

            if (!capitalMap.empty())
            {
                std::string::size_type lastIndex = -1;
                for (std::string::size_type i = 0; i < src.size(); ++i)
                {
                    char ch = src[i];

                    auto chain = capitalMap[ch];
                    if (!chain.empty())
                    {
                        std::string tok;
                        for (auto item : chain)
                        {
                            if (i + item.size() - 1 < src.size())
                            {
                                if (item == src.substr(i, item.size()))
                                {
                                    tok = item;
                                    break;
                                }
                            }
                        }

                        if (!tok.empty())
                        {
                            if (lastIndex == -1)
                            {
                                lastIndex = 0;
                            }

                            arr.push_back(src.substr(lastIndex, i - lastIndex));

                            i += tok.size() - 1;
                            lastIndex = i + 1;
                        }
                    }
                }

                if (lastIndex != -1 && lastIndex < src.size())
                {
                    arr.push_back(src.substr(lastIndex));
                }
            }
        }
    }
    return arr;
}

void Util::throwError(std::string msg)
{
    throw std::logic_error(msg);
}

std::vector<char> Util::readFileSync(std::string filePath)
{
    std::vector<char> chars;

    auto file = std::ifstream(filePath, std::ios::binary);
    if (file.is_open())
    {
        chars = std::vector<char>(file.seekg(0, std::ios::end).tellg(), 0);
        file.seekg(0, std::ios::beg).read(&chars[0], static_cast<std::streamsize>(chars.size()));
        file.close();
    }
    return chars;
}

std::string Util::currentWorkDirectory()
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

bool Util::isDirectory(std::string path)
{
    struct stat buf;
    return lstat(path.c_str(), &buf) < 0 ? false : S_ISDIR(buf.st_mode);
}

std::vector<std::string> Util::filesInTheCurrentDirectory(std::string filePath)
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

std::map<std::string,std::string> Util::Argv2Map(const char * argv[],int len,const std::map<std::string,int> rule)
{
    auto dic = std::map<std::string,std::string>();

    for (int i = 0;i < len;++i)
    {
        auto arg = argv[i];

        if (!arg || strlen(arg) == 0)
        {
            continue;
        }

        auto ite = rule.find(arg);
        if (ite == rule.end())
        {
            continue;
        }

        auto count = ite->second;
        auto content = std::string();

        auto j = i;
        for (;j < i + count && j < len;++j)
        {
            if (!content.empty())
            {
                content += " ";
            }
            content += argv[j];
        }
        i = j;

        dic[arg] = content;
    }

    return dic;
}

std::wstring Util::s2ws(const std::string &s)
{
    auto curLocale = setlocale(LC_ALL, "");
    
    auto _Source = s.c_str();
    auto _Dsize = mbstowcs(nullptr, _Source, 0) + 1;
    auto _Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    auto result = std::wstring(_Dest);
    delete []_Dest;
    
    setlocale(LC_ALL, curLocale);
    return result;
}

unsigned long Util::u_strlen(const char *utf8_str)
{
    unsigned long len = 0;
    
    if (!utf8_str)
    {
        return len;
    }
    
    auto size = strlen(utf8_str);
    if (size == 0)
    {
        return len;
    }
    
    for (auto i = 0;i < size;++i)
    {
        unsigned long tmp = utf8_str[i];
        unsigned int bits[8] = {};
        
        auto idx = 7;
        while (tmp != 0 && idx >= 0)
        {
            bits[idx] = tmp % 2;
            idx--;
            tmp /= 2;
        }
        
        if (bits[0] == 1)
        {
            auto count_one = 1;
            while (bits[count_one] == 1)
            {
                count_one++;
            }
            
            i += (count_one - 1);
        }
        
        len++;
    }
    
    return len;
}
