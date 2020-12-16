#include "connection.h"

Connection::Connection(asio::ip::tcp::socket&& socket__, asio::io_context& ic)
    : socket_(std::move(socket__))
{
    i_cont = &ic;
    listen();
}

Connection::Connection(asio::ip::tcp::socket&& socket__,
    std::function<void(std::vector<char>&& dVec)> onMR, asio::io_context& ic)
    : socket_(std::move(socket__))
{
    i_cont = &ic;
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
        WorkerThread.join();
        WorkerThread = std::thread(
            [this]()
            {
                while (isOpen && outqueue.size() != 0)
                {
                    write();
                }
            }
        );
    }
}

void Connection::write()
{
    if (!socket_.is_open())
    {
        Stop();
        return;
    }
    writeBBuffer = outqueue.pop_front();
    MHeader header;
    header.DataSize = writeBBuffer.size();
    writeHBuffer = std::vector<char>((char*)&header, (char*)&header + sizeof(MHeader));
    boost::system::error_code ec;
    socket_.wait(socket_.wait_write);
    socket_.write_some(asio::buffer(writeHBuffer), ec);
    if (ec)
    {
        std::cout << "Some error occured stoping connection. reason :" << ec.message() << '\n';
        Stop();
        return;
    }
    socket_.wait(socket_.wait_write);
    socket_.write_some(asio::buffer(writeBBuffer), ec);
    if (ec)
    {
        std::cout << "Some error occured stoping connection. reason :" << ec.message() << '\n';
        Stop();
        return;
    }
}

void Connection::listen()
{
    //std::size_T May need to be replaced with size_t
    asio::async_read(socket_, asio::buffer(readHBuffer, sizeof(MHeader)),
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
                asio::async_read(socket_, asio::buffer(readBBuffer, header.DataSize),
                    [this](boost::system::error_code ec, std::size_t length)
                    {
                        if (ec)
                        {
                            Stop();
                            std::cout << "Stopping connection due to " << ec.message() << std::endl;
                            return;
                        }
                        
                        if (useQueueOnMessageReceive)
                        {
                            inqueue.push_back(std::move(readBBuffer));
                        }
                        else
                        {
                            OnMessageReceive(std::move(readBBuffer));
                        }
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
    if (!isOpen)
        return;
    isOpen = false;
    socket_.cancel();
    socket_.close();
    WorkerThread.join();
}

Connection::~Connection()
{
    Stop();
}