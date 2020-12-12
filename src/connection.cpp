#include "connection.h"

Connection::Connection(asio::ip::tcp::socket&& socket__)
    : socket_(std::move(socket__))
{
    listen();
}

Connection::Connection(asio::ip::tcp::socket&& socket__,
    std::function<void(std::vector<char>& dVec)> onMR)
    : socket_(std::move(socket__))
{
    OnMessageReceive = onMR;
    useQueueOnMessageReceive = false;
    listen();
}

void Connection::Send(std::vector<char>&& DVec)
{
    
    //May require a mutex here
    size_t qsize = outqueue.size();
    outqueue.push_back(std::move(DVec));
    //
    if (qsize == 0)
    {
        write();
    }

}

void Connection::write()
{
    writeBBuffer = outqueue.pop_front();
    MHeader header;
    header.DataSize = writeBBuffer.size();
    writeHBuffer = std::vector<char>((char*)&header, (char*)&header + sizeof(MHeader));

    std::cout << "Sending message___\n";
    asio::async_write(socket_,asio::buffer(writeHBuffer, sizeof(MHeader)),
        [this](boost::system::error_code ec, std::size_t length)
        {
            std::cout << "Sending message\n";
            if (ec)
            {
                std::cout << "Stopping connection due to " << ec.message() << std::endl;
                Stop();
                return;
            }
            asio::async_write(socket_,asio::buffer(writeBBuffer, writeBBuffer.size()),
                [this](boost::system::error_code ec, std::size_t length)
                {
                    if (ec)
                    {
                        std::cout << "Stopping connection due to " << ec.message() << std::endl;
                        Stop();
                        return;
                    }
                    if (outqueue.size() > 0)
                    {
                        write();
                    }
                });
        });
}

void Connection::listen()
{
    //std::size_T May need to be replaced with size_t
    asio::async_read(socket_,asio::buffer(readHBuffer, sizeof(MHeader)),
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (ec)
            {
                std::cout << "Stopping connection due to " << ec.message() << std::endl;
                Stop();
                return;
            }
            MHeader header = *(MHeader*)(readHBuffer.data());
            if (header.DataSize > 0)
            {
                readBBuffer = std::vector<char>(header.DataSize, 0);
                std::cout << readBBuffer.size() << std::endl;
                asio::async_read(socket_,asio::buffer(readBBuffer, header.DataSize),
                    [this](boost::system::error_code ec, std::size_t length)
                    {
                        std::cout << "reaading message!\n";
                        for (char c : readBBuffer)
                        {
                            std::cout << c;
                        }
                        std::cout << std::endl;
                        if (ec)
                        {
                            Stop();
                            std::cout << "Stopping connection due to " << ec.message() << std::endl;
                            return;
                        }
                        /*
                        if (useQueueOnMessageReceive)
                        {
                            inqueue.push_back(std::move(readBBuffer));
                        }
                        else
                        {
                            OnMessageReceive(readBBuffer);
                        }*/
                        listen();
                    });
            }
            else
            {
                listen();
            }

        });
}

void Connection::Stop()
{
    socket_.cancel();
    socket_.close();
}

Connection::~Connection()
{
    Stop();
}