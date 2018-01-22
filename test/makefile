cc = g++

socket = ../src/Socket/*.cpp
http = ../src/HTTP/*.cpp
utility = ../src/Utility/*.cpp

openssl_lib = /usr/local/Cellar/openssl/1.0.2n/lib
openssl_include = /usr/local/Cellar/openssl/1.0.2n/include
openssl_flag = $(openssl_lib)/*.a -L $(openssl_lib) -I$(openssl_include)

cpp_flag = -std=c++14 -g

build-request-mac:
	$(cc) test.cpp $(cpp_flag) $(socket) $(http) $(utility) $(openssl_flag)

build-server-mac:
	$(cc) test.cpp $(socket) $(http) $(utility) $(openssl_lib)/*.a -L $(openssl_lib) -I$(openssl_include) $(cpp_flag)

build-server-linux:
	$(cc) test.cpp $(socket) $(http) $(utility) -lssl -lcrypto -lpthread $(cpp_flag)

clean:
	rm *.out && rm -r a.out.dSYM