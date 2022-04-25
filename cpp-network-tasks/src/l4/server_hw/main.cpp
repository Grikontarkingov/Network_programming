#include <iostream>
#include <memory>
#include <string>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
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
        send();
    }


private:
    TcpConnection(boost::asio::io_context& io_context)
        : socket_(io_context) {}


    bool send_file_offset(const std::string& file_path, std::vector<char>& buffer)
    {
        std::ifstream file_stream(file_path, std::ifstream::binary);

        if (!file_stream) return false;

        std::cout << "Sending file " << file_path << "..." << std::endl;
        while (file_stream)
        {
            file_stream.read(&buffer[0], buffer.size());
        }

        return true;
    }

    void handle_write(const boost::system::error_code& error, size_t bytes_transfered)
    {
        if(!error)
        {
            std::cout << "Bytes transfered: " << bytes_transfered << std::endl;
        }
    }

    void send()
    {
        auto s = shared_from_this();
        std::vector<char> buffer(buffer_size);
        auto path = read();

        send_file_offset(path, buffer);

        const size_t offset = 20;
        size_t pos = 0;

        const auto size = buffer.size();
        std::string transfer;

        while(pos != size) {
            transfer = &(buffer.data()[0]) + pos;
            boost::asio::async_write(socket_, boost::asio::buffer(transfer),
                                     [s](const boost::system::error_code& error, size_t bytes_transfered)
                                     {
                                         s->handle_write(error, bytes_transfered);
                                     }
            );

            if (pos + offset > size)
            {
                pos += (size - pos);
            }
            else
            {
                pos += offset;
            }

            transfer.clear();
        }
    }

    std::string read()
    {
        std::array<char, MAX_PATH + 1> buffer;
        size_t recv_bytes = 0;
        const auto size = buffer.size() - 1;
        auto s = shared_from_this();

        std::cout << "Reading user request..." << std::endl;
        socket_.async_read_some(boost::asio::buffer(buffer, size),
                                [s](const boost::system::error_code& error, size_t bytes_transfered)
                                {
                                    s->handle_read(error, bytes_transfered);
                                }
        );

        buffer[recv_bytes] = '\0';

        auto result = std::string(buffer.begin(), buffer.begin() + recv_bytes);
        std::cout << "Request = \"" << result << "\"" << std::endl;

        return result;
    }

    void handle_read(const boost::system::error_code& error, size_t bytes_read)
    {
        if(!error && bytes_read)
        {
            std::cout << "Bytes receive = " << bytes_read << std::endl;
        }

        send();
    }

private:
    tcp::socket socket_;
    const size_t buffer_size = 4096;
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
        TcpConnection::pointer new_connection =
                TcpConnection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
                               [this, new_connection](const boost::system::error_code& error)
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

