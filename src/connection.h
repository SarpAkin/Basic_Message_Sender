#ifndef CONNECTION_H
#define CONNECTION_H

#include <functional>
#include <iostream>
#include <vector>
#include <thread>

#include <boost/asio/ts/internet.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio.hpp>

#include "Tsafe_queue.h"

namespace asio = boost::asio;

struct MHeader
{
    uint32_t DataSize;
};

class Connection
{
private:
    std::thread WorkerThread;
    asio::ip::tcp::socket socket_;
    std::function<void(std::vector<char>& dVec)> OnMessageReceive;
    bool useQueueOnMessageReceive = true;

    T_queue<std::vector<char>> inqueue;
    T_queue<std::vector<char>> outqueue;

    std::vector<char> readHBuffer = std::vector<char>(sizeof(MHeader));
    std::vector<char> readBBuffer;

    std::vector<char> writeBuffer;

private:
    void listen();
    void write();

public:
    Connection(asio::ip::tcp::socket&& socket__);
    Connection(asio::ip::tcp::socket&& socket__, std::function<void(std::vector<char>& dVec)>);
    ~Connection();

    void Stop();
    void Send(std::vector<char>&& DVec);
};

Connection::Connection(asio::ip::tcp::socket&& socket__)
    : socket_(std::move(socket__))
{

}

Connection::Connection(asio::ip::tcp::socket&& socket__,
    std::function<void(std::vector<char>& dVec)> onMR)
    : socket_(std::move(socket__))
{
    OnMessageReceive = onMR;
    useQueueOnMessageReceive = false;
}

void Connection::Send(std::vector<char>&& DVec)
{
    //May require a mutex here
    size_t qsize = outqueue.size();
    outqueue.push_back(std::move(DVec));
    //
    if (qsize == 0)
    {
        write();
    }

}

void Connection::write()
{
    auto dVec = outqueue.pop_front();
    MHeader header;
    header.DataSize = dVec.size();
    writeBuffer = std::vector<char>((char*)&header,(char*)&header + sizeof(MHeader));
    writeBuffer.insert(writeBuffer.end(),dVec.begin(),dVec.end());
    socket_.async_write_some(asio::buffer(writeBuffer, sizeof(MHeader)),
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (ec)
            {
                std::cout << "Stopping connection due to " << ec.message() << std::endl;
                Stop();
                return;
            }
            if(outqueue.size() > 0)
            {
                write();
            }            
        });
}

void Connection::listen()
{
    //std::size_T May need to be replaced with size_t
    socket_.async_read_some(asio::buffer(readHBuffer, sizeof(MHeader)),
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (ec)
            {
                std::cout << "Stopping connection due to " << ec.message() << std::endl;
                Stop();
                return;
            }
            MHeader header = *(MHeader*)(readHBuffer.data());
            if (header.DataSize > 0)
            {
                readBBuffer = std::vector<char>(header.DataSize, 0);
                socket_.async_read_some(asio::buffer(readBBuffer, header.DataSize),
                    [this](boost::system::error_code ec, std::size_t length)
                    {
                        if (ec)
                        {
                            Stop();
                            std::cout << "Stopping connection due to " << ec.message() << std::endl;
                            return;
                        }
                        if (useQueueOnMessageReceive)
                        {
                            inqueue.push_back(std::move(readBBuffer));
                        }
                        else
                        {
                            OnMessageReceive(readBBuffer);
                        }
                        listen();
                    });
            }
            else
            {
                listen();
            }

        });
}



void Connection::Stop()
{
    socket_.cancel();
    socket_.close();
}

Connection::~Connection()
{
    Stop();
}



#endif