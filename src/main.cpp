#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <boost/asio/ts/internet.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio.hpp>

#include "connection.h"

namespace asio = boost::asio;

void testF()
{
    auto func = []()
    {

    };
}

const char* message = "Hello client!";
const int messagesize = 14;

int main()
{

    return 0;
    asio::io_context Sic;
    asio::ip::tcp::acceptor connection_acceptor(Sic, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 30020));
    connection_acceptor.async_accept(
        [&Sic](boost::system::error_code ec, asio::ip::tcp::socket _socket)
        {
            std::cout << "Connecting to a client\n";
            Connection connection(std::move(_socket), Sic);
            std::cout << "Connected to client\n";
            connection.Send(std::vector<char>(message, message + messagesize));
        }
    );
    auto ic_thread = std::thread([&Sic]() {
        Sic.run();
        });

    asio::io_context ic;
    asio::ip::tcp::socket socket_(ic);
    auto endpoint = asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 30020);
    auto lambda = [](std::vector<char> dVec)
    {
        for (char c : dVec)
        {
            std::cout << c;
        }
        std::cout << std::endl;
    };
    boost::system::error_code ec;
    socket_.connect(endpoint, ec);
    Connection connection(std::move(socket_), lambda, ic);

    std::cout << ec.message() << std::endl;
    std::cout << "end\n";

    std::string str;
    std::cin >> str;

    Sic.stop();
    ic_thread.join();
    return 0;
}

int _main()
{
    boost::system::error_code ec;
    asio::io_context ic;

    auto endpoint = asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1", ec), 30020);
    auto socket = asio::ip::tcp::socket(ic);

    socket.connect(endpoint, ec);

    std::string data = "s";
    socket.write_some(asio::buffer(data));

    socket.wait(socket.wait_read);
    size_t bytes = socket.available(ec);

    std::cout << bytes << std::endl;
    std::cout << ec.message() << std::endl;

    if (1)
    {
        std::vector<char> Rdata(bytes);
        socket.read_some(asio::buffer(Rdata));
        Rdata.push_back(0);
        for (char c : Rdata)
        {
            std::cout << c;
        }

    }
    return 0;
}