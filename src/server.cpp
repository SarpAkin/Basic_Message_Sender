#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

#include "connection.h"
namespace asio = boost::asio;

struct Client
{
    uint32_t id;
    std::unique_ptr<Connection> connection;
};

const auto lambda = [](std::vector<char> dVec)
{
    for (char c : dVec)
    {
        std::cout << c;
    }
    std::cout << std::endl;
};

class Server
{
public:
    asio::io_context ic;
private:
    std::vector<Client> clients;
    std::mutex clientV_mut;
    uint32_t clCounter = 0;
    std::thread ic_thread;
    asio::ip::tcp::acceptor connection_acceptor;

public:
    Server(uint16_t portNum);
    ~Server();

    void Stop();
    void AddConnection(std::unique_ptr<Connection> client);
    void AcceptConnections();
};

Server::Server(uint16_t portNum)
    : connection_acceptor(ic, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), portNum))
{
    std::cout << "Server Started\n";
    AcceptConnections();
    try
    {
        ic_thread = std::thread([this]() {
            ic.run();
            });
    }
    catch (const boost::system::error_code& ec)
    {
        std::cerr << ec.message() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::AddConnection(std::unique_ptr<Connection> client)
{
    Client cl;
    cl.connection = std::move(client);
    clientV_mut.lock();
    cl.id = clCounter;
    clCounter++;
    clients.push_back(std::move(cl));
    std::string str = "Succesfully Connected To server " + std::to_string(clients.back().id);
    std::vector<char> dVec(str.begin(), str.end());
    clients.back().connection.get()->Send(std::move(dVec));
    clientV_mut.unlock();
}

void Server::AcceptConnections()
{
    connection_acceptor.async_accept([this](boost::system::error_code ec, asio::ip::tcp::socket _socket)
        {
            if (!ec)
            {
                std::cout << "Adding Connection to server!" << std::endl;
                AddConnection(std::make_unique<Connection>(std::move(_socket),lambda,ic));
            }
            else
            {
                //TODO handle error
                std::cout << ec.message() << std::endl;
            }
            AcceptConnections();
        });
}

void Server::Stop()
{
    connection_acceptor.cancel();
    connection_acceptor.close();
    clientV_mut.lock();
    for (auto& cl : clients)
    {
        cl.connection->Stop();
    }
    clientV_mut.unlock();
    ic.stop();
    ic_thread.join();
    std::cout << "Server Stopped\n";
}

Server::~Server()
{
    Stop();
}

int main()
{
    try
    {
        Server server(30020);
        std::cin.get();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Server Stopped\n";
    std::cin.get();
    //Server server(30020);
}