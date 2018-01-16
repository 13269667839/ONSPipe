cc = g++
socket = ../src/Socket/*.cpp
http = ../src/HTTP/*.cpp
utility = ../src/Utility/*.cpp
openssl_a = ../src/ThirdParty/openssl
cpp_flag = -std=c++11 -g

build-server-mac:
	$(cc) test.cpp $(socket) $(http) $(utility) $(openssl_a)/*.a -L $(openssl_a) -I../src/ThirdParty/openssl/include -lpthread $(cpp_flag)

build-server-linux:
	$(cc) test.cpp $(socket) $(http) $(utility) -lssl -lcrypto -lpthread $(cpp_flag)

clean:
	rm *.out && rm -r a.out.dSYM