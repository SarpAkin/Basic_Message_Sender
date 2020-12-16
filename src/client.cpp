#include <iostream>
#include <string>
#include <vector>

#include "connection.h"
namespace asio = boost::asio;

const char* ServerIp = "127.0.0.1";

int main()
{
    try
    {
        asio::io_context ic;


        asio::ip::tcp::socket socket_(ic);
        auto endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(ServerIp), 30020);
        auto lambda = [](std::vector<char> dVec)
        {
            for (char c : dVec)
            {
                std::cout << c;
            }
            std::cout << std::endl;
        };
        socket_.connect(endpoint);
        Connection connection(std::move(socket_), lambda, ic);

        auto ic_thread = std::thread([&ic]()
            {
                ic.run();
            });
        
        while (1)
        {
            std::string str;
            std::cin >> str;
            if (str == "exit")
            {
                break;
            }
            std::vector<char> dVec = std::vector<char>(str.begin(), str.end());
            connection.Send(std::move(dVec));
        }

        
        ic.stop();
        connection.Stop();
        ic_thread.join();

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