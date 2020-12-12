#include "Socket.h"

Socket::Socket(const char* ip, uint16_t port)
    : socket_(ic)
{
    boost::system::error_code ec;
    auto endpoint = asio::ip::tcp::endpoint(
        asio::ip::make_address(ip, ec),
        port);
    socket_.connect(endpoint, ec);
    if (ec)
        throw ec;
}

Socket::Socket(asio::ip::tcp::socket&& socket)
    : socket_(std::move(socket))
{

}




void Socket::Listen(std::function<void(std::vector<char>& dataVec)> onPackageRecevie)
{
    if (isListening == true)
        return;
    ListenThread = std::thread(&Socket::Listen_, this, onPackageRecevie);
}

void Socket::Listen_(std::function<void(std::vector<char>& dataVec)> onPackageRecevie)
{
    try
    {
        isListening = true;
        while (isListening)
        {
            auto Hvec = std::vector<char>(sizeof(Header));
            socket_.read_some(asio::buffer(Hvec));
            Header header = *(Header*)Hvec.data();
            auto Dvec = std::vector<char>(header.DataSize);
            socket_.read_some(asio::buffer(Dvec));
            onPackageRecevie(Dvec);
        }
        std::cout << "Connection closed\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        isListening = false;
    }
}


void Socket::Write(std::vector<char>& dataVec)
{
    try
    {
        Header header;
        header.DataSize = dataVec.size();
        auto Dvec = std::vector<char>((char*)&header, (char*)&header + sizeof(Header));
        Dvec.insert(Dvec.end(), dataVec.begin(), dataVec.end());
        socket_.write_some(asio::buffer(Dvec));
    }
    catch (const boost::system::error_code& e)
    {
        std::cerr << e.message() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Socket::Write(std::string& dataVec)
{
    try
    {
        Header header;
        header.DataSize = dataVec.size();
        auto Dvec = std::vector<char>((char*)&header, (char*)&header + sizeof(Header));
        Dvec.insert(Dvec.end(), dataVec.begin(), dataVec.end());
        socket_.write_some(asio::buffer(Dvec));
    }
    catch (const boost::system::error_code& e)
    {
        std::cerr << e.message() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

Socket::~Socket()
{
    isListening = false;
    socket_.close();
    ListenThread.join();
}