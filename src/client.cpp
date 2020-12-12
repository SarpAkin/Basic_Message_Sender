#include <iostream>
#include <string>
#include <vector>

#include "Socket.h"
namespace asio = boost::asio;

const char* ServerIp = "127.0.0.1";

int main()
{
    try
    {
        Socket Lsocket(ServerIp, 30020);
        //Socket Wsocket(ServerIp, 30020);
        Lsocket.Listen([](std::vector<char>& dVec)
            {
                std::cout << dVec.size() << std::endl;
                for (char c : dVec)
                    std::cout << c;

                std::cout << std::endl;
            });

        std::cout << "Connecting to server!\n";
        asio::io_context ic;
        //asio::ip::tcp::acceptor connection_acceptor(ic, asio::ip::tcp::endpoint(asio::ip::make_address(ServerIp), 30020));
        asio::ip::tcp::acceptor connection_acceptor(ic, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 30021));
        auto soc_ = connection_acceptor.accept();
        Socket Wsocket(std::move(soc_));
        std::cout << "Connected to server!\n";

        while (true)
        {
            std::string inp;
            std::cin >> inp;
            if (inp == "exit")
                exit(0);
            Wsocket.Write(inp);
        }

        std::cin.get();
    }
    catch (const boost::system::error_code& ec)
    {
        std::cerr << ec.message() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }


    return 0;
}