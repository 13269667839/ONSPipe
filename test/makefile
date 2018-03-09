OS = $(shell uname -s)

socket = ../src/Socket/*.cpp
http = ../src/HTTP/*.cpp
utility = ../src/Utility/*.cpp
json = ../src/JSON/*.cpp
xml = ../src/XML/*.cpp
sqlite = ../src/SQLite/*.cpp
websocket = ../src/WebSocket/*.cpp

file_flag = $(socket) $(http) $(utility) $(json) $(xml) $(sqlite) $(websocket)

ifeq ($(OS),Darwin)
	CC = clang++

	openssl_lib = /usr/local/Cellar/openssl/1.0.2n/lib
	openssl_include = /usr/local/Cellar/openssl/1.0.2n/include
	openssl_flag = $(openssl_lib)/*.a -L $(openssl_lib) -I $(openssl_include)

	cpp_flag = -std=c++14 -g -W -lpthread -lz -lsqlite3

	extra_flag = $(openssl_flag)
endif

ifeq ($(OS),Linux)
	CC = g++

	cpp_flag = -std=c++14 -g -W -lpthread -lz -lsqlite3 -lssl -lcrypto
endif

build:
	$(CC) test.cpp $(cpp_flag) $(file_flag) $(extra_flag)

clean:
	rm a.out
	rm -r a.out.dSYM