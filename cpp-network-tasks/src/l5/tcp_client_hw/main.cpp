#include <iostream>
#include <fstream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    boost::asio::io_context io_context;

    const int port { std::stoi(argv[2]) };
    const std::string host_name { argv[1] };

    tcp::endpoint ep( boost::asio::ip::address::from_string(host_name), port);
    tcp::socket sock(io_context);

    sock.connect(ep);

    std::string file_path_buffer;
    std::cout << "Set file name for downloading: " << std::endl;
    std::cout << ">$ ";
    std::cin >> file_path_buffer;

    boost::system::error_code ec;
    size_t write_bytes = sock.write_some(boost::asio::buffer(file_path_buffer), ec);
    if(!ec)
    {
        std::cout << "Bytes transfer: " << write_bytes << std::endl;
    }

    std::vector<char> buffer;
    const size_t buffer_size = 128;
    buffer.resize(buffer_size);

    size_t read_bytes = sock.receive(boost::asio::buffer(&buffer[0], buffer_size), 0, ec);
    if(!ec)
    {
        std::cout << "Bytes receive: " << read_bytes << std::endl;
    }

    if(read_bytes <= 0)
    {
        std::cout << "Bytes receive: " << read_bytes << std::endl;
        return EXIT_SUCCESS;
    }

    std::ofstream file_stream(file_path_buffer, std::ios::binary);
    if(!file_stream)
    {
        std::cerr << "File stream error" << std::endl;
        return EXIT_FAILURE;
    }

    file_stream.write(&buffer[0], buffer_size);

    return EXIT_SUCCESS;
}
