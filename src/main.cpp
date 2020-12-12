#include <functional>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

namespace asio = boost::asio;

void testF()
{
    auto func = [](){

    };
    
}

int main()
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
        for(char c: Rdata)
        {
            std::cout << c;
        }

    }
    return 0;
}