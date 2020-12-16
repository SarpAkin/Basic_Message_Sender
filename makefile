CFLAGS = -std=c++17 -pthread -lstdc++ -g
CC = gcc

all:client server

main:src/main.cpp src/connection.cpp src/connection.h
	$(CC) -o main src/main.cpp  src/connection.cpp $(CFLAGS)

client:src/client.cpp src/connection.cpp
	$(CC) -o client src/client.cpp  src/connection.cpp $(CFLAGS)

server:src/server.cpp  src/connection.cpp
	$(CC) -o server src/server.cpp  src/connection.cpp $(CFLAGS)