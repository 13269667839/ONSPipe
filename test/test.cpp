#include <iostream>
#include "../src/HTTP/HTTPClient.hpp"
using namespace std;

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
