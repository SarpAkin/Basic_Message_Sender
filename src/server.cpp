#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

#include "Socket.h"
namespace asio = boost::asio;

struct Client
{
    uint32_t id;
    std::unique_ptr<Socket> Csocket;
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

    void AddConnection(std::unique_ptr<Socket> client);
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

void Server::AddConnection(std::unique_ptr<Socket> client)
{
    Client cl;
    cl.Csocket = std::move(client);
    clientV_mut.lock();
    cl.id = clCounter;
    clCounter++;
    clients.push_back(std::move(cl));
    std::string str = "Succesfully Connected To server " + std::to_string(clients.back().id);
    auto socptr = clients.back().Csocket.get();
    std::thread([socptr, str]() {
        auto str_ = std::move(str);
        socptr->Write(str_);
        }).detach();
        clientV_mut.unlock();
}

void Server::AcceptConnections()
{
    connection_acceptor.async_accept([this](boost::system::error_code ec, asio::ip::tcp::socket _socket)
        {
            if (!ec)
            {
                std::cout << "Adding Connection to server!" << std::endl;
                AddConnection(std::make_unique<Socket>(std::move(_socket)));
            }
            else
            {
                //TODO handle error
                std::cout << ec.message() << std::endl;
            }
            AcceptConnections();
        });
}
Server::~Server()
{
    ic_thread.join();
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