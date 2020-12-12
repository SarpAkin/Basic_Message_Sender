#ifndef Socket_H
#define Socket_H

#include <functional>
#include <iostream>
#include <vector>
#include <thread>

#include <boost/asio/ts/internet.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio.hpp>

namespace asio = boost::asio;

struct Header
{
    uint32_t DataSize;
};

class Socket
{
public:
    bool isListening;
    //boost::system::error_code ec;
private:
    std::thread ListenThread;
    asio::io_context ic;
    asio::ip::tcp::socket socket_;
public:
    Socket(const char* ip, uint16_t port);
    Socket(asio::ip::tcp::socket&& socket);
    ~Socket();

    void Listen(std::function<void(std::vector<char>& dataVec)> onPackageRecevie);
    void Write(std::vector<char>& dataVec);
    void Write(std::string& dataVec);
private:
    void Listen_(std::function<void(std::vector<char>& dataVec)> onPackageRecevie);
};

#endif