cc = g++

socket = ../src/Socket/*.cpp
http = ../src/HTTP/*.cpp
utility = ../src/Utility/*.cpp
json = ../src/JSON/*.cpp
xml = ../src/XML/*.cpp

file_flag = $(socket) $(http) $(utility) $(json) $(xml)

openssl_lib = /usr/local/Cellar/openssl/1.0.2n/lib
openssl_include = /usr/local/Cellar/openssl/1.0.2n/include
openssl_flag = $(openssl_lib)/*.a -L $(openssl_lib) -I$(openssl_include)

cpp_flag = -std=c++14 -g -W -lpthread -lz

build-mac:
	$(cc) test.cpp $(cpp_flag) $(file_flag) $(openssl_flag)

build-linux:
	$(cc) test.cpp $(file_flag) -lssl -lcrypto $(cpp_flag)

clean:
	rm *.out && rm -r a.out.dSYM