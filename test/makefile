cc = g++
socket = ../src/Socket/*.cpp
http = ../src/HTTP/*.cpp
utility = ../src/Utility/*.cpp
openssl_a = ../src/ThirdParty/openssl

build-server-mac:
	$(cc) test.cpp $(socket) $(http) $(utility) $(openssl_a)/*.a -L $(openssl_a) -I../src/ThirdParty/openssl/include -lpthread -O3 -std=c++11

build-server-linux:
	$(cc) test.cpp $(socket) $(http) $(utility) -lssl -lcrypto -lpthread -O3 -std=c++11

clean:
	rm *.out