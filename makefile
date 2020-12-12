CFLAGS = -std=c++17 -pthread -lstdc++
CC = clang

all:client server

client:src/client.cpp src/Socket.cpp
	$(CC) -o client src/client.cpp src/Socket.cpp $(CFLAGS)

server:src/server.cpp src/Socket.cpp
	$(CC) -o server src/server.cpp src/Socket.cpp $(CFLAGS)