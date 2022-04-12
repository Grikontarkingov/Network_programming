#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>



int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[1]) };

    socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, IPPROTO_TCP};
    int newsock = {0};

    std::cout << "Starting echo server on the port " << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr =
    {
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    listen(sock, 5);

    char buffer[256];

    // socket address used to store client address
    struct sockaddr_storage client_addr = {0};
    socklen_t client_len = sizeof(client_addr);
    ssize_t recv_len = 0;

    std::cout << "Running echo server...\n" << std::endl;
    char client_address_buf[INET_ADDRSTRLEN];

    while (true)
    {
        newsock = accept(sock, reinterpret_cast<sockaddr *>(&client_addr),
                           &client_len);

        if (newsock < 0)
        {
            throw std::runtime_error("ERROR on accept");
        }

        while(true)
        {
            // Read content into buffer from an incoming client.
            recv_len = recv(newsock, buffer, sizeof(buffer), 0);

            if (recv_len > 0)
            {
                buffer[recv_len] = '\0';
                std::cout
                        << "Client with address "
                        << inet_ntop(AF_INET, &(reinterpret_cast<const sockaddr_in* const> (&client_addr)->sin_addr), client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                        << ":" << ntohs((reinterpret_cast<const sockaddr_in* const>(&client_addr)->sin_port))
                        << " sent datagram "
                        << "[length = "
                        << recv_len
                        << "]:\n'''\n"
                        << buffer
                        << "\n'''"
                        << std::endl;
                //if ("exit" == command_string) run = false;
                //send(sock, &buf, readden, 0);

/*            std::string command_string = {buffer, 0, len};
            rtrim(command_string);
            std::cout << command_string << std::endl;
*/
                // Send same content back to the client ("echo").
                send(newsock, buffer, recv_len, 0);
            }

            std::cout << std::endl;
        }
    }

    return EXIT_SUCCESS;
}

