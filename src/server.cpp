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
    void AddConnection(Client client);
    void AcceptConnections();

    bool SendMessage(std::vector<char> mVec, int clientNum);
    bool SendMessageAll(std::vector<char> mVec);

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

void Server::AddConnection(Client cl)
{
    clientV_mut.lock();
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
                Client client;
                int cnum = clCounter;
                clCounter++;
                client.id = cnum;
                client.connection = std::make_unique<Connection>(std::move(_socket),
                    [this, cnum](std::vector<char>&& dVec)
                    {

                        std::string message("[");
                        message += std::to_string(cnum);
                        message += ']';
                        message.append(dVec.begin(), dVec.end());
                        std::cout << message << std::endl;
                        SendMessageAll(std::vector<char>(message.begin(),message.end()));
                    }
                , ic);
                AddConnection(std::move(client));
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

bool Server::SendMessage(std::vector<char> mVec, int clientId)
{
    clientV_mut.lock();

    for (auto& cl : clients)
    {
        if (cl.id == clientId)
        {
            if (cl.connection->isopen())
            {
                cl.connection->Send(std::move(mVec));
                clientV_mut.unlock();//Unlock it on return
                return true;
            }
        }
    }

    clientV_mut.unlock();
    return false;
}

bool Server::SendMessageAll(std::vector<char> mVec)
{
    clientV_mut.lock();

    std::cout << clients.size() << '\n';
    for (auto& cl : clients)
    {
        if (cl.connection->isopen())
        {
            auto mCopy = mVec;
            cl.connection->Send(std::move(mCopy));
        }
    }

    clientV_mut.unlock();
    return true;
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
    //Server server(30020);
}