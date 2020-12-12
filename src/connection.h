#ifndef CONNECTION_H
#define CONNECTION_H

#include <functional>
#include <iostream>
#include <vector>
#include <thread>

#include <boost/asio/ts/internet.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio.hpp>

namespace asio = boost::asio;

class Connection
{
private:
    std::thread WorkerThread;
    asio::ip::tcp::socket socket_;
public:
    Connection(asio::ip::tcp::socket&& socket__);
    ~Connection();
};

Connection::Connection(asio::ip::tcp::socket&& socket__)
: socket_(std::move(socket__))
{
    
}

Connection::~Connection()
{

}



#endif