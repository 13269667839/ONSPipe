#include "Base64.hpp"
#include <cctype>
#include <deque>
#include "Util.hpp"

static const std::string encode_convert_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline int two_square(int count)
{
    int res = 1;
    while (count >= 1)
    {
        res *= 2;
        count--;
    }
    return res;
}

std::string Base64::encode(std::string code)
{
    auto b64code = std::string();
    
    if (code.empty())
    {
        return b64code;
    }
    
    int remain = code.size() % 3;
    if (remain != 0)
    {
        int count = 3 - remain;
        remain = count;
        while (count > 0)
        {
            code += '\0';
            count--;
        }
    }
    
    auto group = std::deque<int>();

    for (int ch : code)
    {
        if (!isascii(ch))
        {
            Util::throwError("character must be ascii in base 64 encoding");
        }
        
        int bit[8] = {};
        
        int idx = 8;
        while (ch / 2 != 0)
        {
            bit[idx - 1] = ch % 2;
            idx--;
            ch /= 2;
            
            if (ch == 1)
            {
                bit[idx - 1] = 1;
            }
        }
        
        for (int b : bit)
        {
            group.push_back(b);
        }
    }
    
    int power = 5;
    int number = 0;
    long misTokBorder = remain > 0?group.size() - remain * 6:-1;
    for (auto i = 0;i < group.size();++i)
    {
        number += group[i] * two_square(power);
        
        power--;
        
        if ((i + 1) % 6 == 0)
        {
            if (misTokBorder != -1 && i > misTokBorder)
            {
                b64code += '=';
            }
            else
            {
                b64code += encode_convert_table[number];
            }
            
            number = 0;
            power = 5;
        }
        
    }
    
    return b64code;
}

static int b64_decode_table(char ch)
{
    int number = -1;
    
    if (isupper(ch))
    {
        number = ch - 'A';
    }
    else if (islower(ch))
    {
        number = ch - 'a' + 26;
    }
    else if (isnumber(ch))
    {
        number = ch - '0' + 52;
    }
    else if (ch == '+')
    {
        number = 62;
    }
    else if (ch == '/')
    {
        number = 63;
    }
    
    return number;
}

std::string Base64::decode(std::string code)
{
    auto original_str = std::string();
    if (code.empty())
    {
        return original_str;
    }
    
    auto bits = std::deque<int>();
    for (char ch : code)
    {
        if (ch == '=')
        {
            bits.push_back(0);
            bits.push_back(0);
            break;
        }
        else
        {
            int num = b64_decode_table(ch);
            if (!(num >= 0 && num <= 63))
            {
                break;
            }
            
            int bit[8] = {};
            int idx = 8;
            while (num / 2 != 0)
            {
                bit[idx - 1] = num % 2;
                idx--;
                num /= 2;
                
                if (num == 1)
                {
                    bit[idx - 1] = 1;
                }
            }
            
            for (idx = 2;idx < 8;++idx)
            {
                bits.push_back(bit[idx]);
            }
        }
        
        if (bits.size() >= 8)
        {
            int num = 0;
            int power = 8;
            while (power > 0)
            {
                num += bits[0] * two_square(--power);
                bits.pop_front();
            }
            
            if (!isascii(num))
            {
                Util::throwError("character must be ascii in base 64 encoding");
            }
            
            original_str += num;
        }
    }
    
    if (bits.size() >= 8)
    {
        int num = 0;
        int power = 8;
        while (power > 0)
        {
            num += bits[0] * two_square(--power);
            bits.pop_front();
        }
        
        if (!isascii(num))
        {
            Util::throwError("character must be ascii in base 64 encoding");
        }
        
        original_str += num;
    }
    
    return original_str;
}
