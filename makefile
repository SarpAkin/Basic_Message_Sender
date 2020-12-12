CFLAGS = -std=c++17 -pthread -lstdc++ -g
CC = gcc

all:client server

client:src/client.cpp src/connection.cpp
	$(CC) -o client src/client.cpp  src/connection.cpp $(CFLAGS)

server:src/server.cpp  src/connection.cpp
	$(CC) -o server src/server.cpp  src/connection.cpp $(CFLAGS)