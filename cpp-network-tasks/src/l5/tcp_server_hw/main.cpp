#include <iostream>
#include <memory>
#include <string>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <fstream>

#if !defined(MAX_PATH)
#   define MAX_PATH (256)
#endif

using boost::asio::ip::tcp;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    typedef std::shared_ptr<TcpConnection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new TcpConnection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        read();
    }


private:
    TcpConnection(boost::asio::io_context& io_context)
        : socket_(io_context) {}

    void send()
    {
        auto s = shared_from_this();
        buffer_.resize(buffer_size);

        std::ifstream file_stream(file_path_buffer, std::ios::binary);

        if (!file_stream)
        {
            std::cerr << "File not found" << std::endl;
            return;
        }

        std::cout << "Sending file " << file_path_buffer << "..." << std::endl;

        while(file_stream)
        {
            file_stream.read(&buffer_[0], buffer_.size());
        }

        boost::asio::async_write(socket_, boost::asio::buffer(&buffer_[0], buffer_size),
                                 [s](const boost::system::error_code& error, size_t bytes_transfered)
                                 {
                                     s->handle_write(error, bytes_transfered);
                                 }
        );
    }

    void handle_write(const boost::system::error_code& error, size_t bytes_transfered)
    {
        if(!error)
        {
            std::cout << "Bytes transfered: " << bytes_transfered << std::endl;
        }
    }

    void read()
    {
        file_path_buffer.resize(9);
        auto s = shared_from_this();

        std::cout << "Reading user request..." << std::endl;
        boost::asio::async_read(socket_, boost::asio::buffer(&file_path_buffer[0], file_path_buffer.size()),
                                 [s](const boost::system::error_code& error, size_t bytes_transfered)
                                 {
                                    std::cout << "Request = \"" << s->file_path_buffer << "\"" << std::endl;
                                    s->handle_read(error, bytes_transfered);
                                    s->send();
                                 }
        );
    }

    void handle_read(const boost::system::error_code& error, size_t bytes_transfered)
    {
        if(!error)
        {
            std::cout << "Bytes receive = " << bytes_transfered << std::endl;
            return;
        }
    }

private:
    std::string file_path_buffer;
    std::vector<char> buffer_;
    tcp::socket socket_;
    const size_t buffer_size = 128;
};

class TcpServer
{
public:

    TcpServer(boost::asio::io_context& io_context, const int port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        TcpConnection::pointer new_connection = TcpConnection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
                               [this, new_connection] (const boost::system::error_code& error)
                               {
                                    this->handle_accept(new_connection, error);
                               }
        );
    }

    void handle_accept(TcpConnection::pointer new_connection,
                       const boost::system::error_code& error)
    {
        if(!error)
        {
            new_connection->start();
        }

        start_accept();
    }

private:
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        boost::asio::io_context io_context;

        std::cout << "Starting tcp server on the port " << argv[1] << "...\n";

        TcpServer server(io_context, std::stoi(argv[1]));

        std::cout << "Running tcp server...\n" << std::endl;
        io_context.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
