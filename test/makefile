OS = $(shell uname -s)

entry = test.cpp

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

	openssl_lib = /usr/local/Cellar/openssl/1.0.2o_1/lib
	openssl_include = /usr/local/Cellar/openssl/1.0.2o_1/include
	openssl_flag = $(openssl_lib)/*.a -L $(openssl_lib) -I $(openssl_include)

	cpp_flag = -std=c++14 -g -W -lz -lsqlite3

	extra_flag = $(openssl_flag)
endif

ifeq ($(OS),Linux)
	CC = g++

	cpp_flag = -std=c++14 -g -W -lz -lsqlite3 -lssl -lcrypto
endif

build:
	$(CC) $(entry) $(file_flag) $(cpp_flag) $(extra_flag)

clean:
	rm a.out
	rm -r a.out.dSYM