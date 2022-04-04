#include <iostream>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[2]) };

    socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

    std::cout << "Starting echo client on the address:port " << argv[1] << ":" << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    const size_t SIZE = 256;
    char send_buf[SIZE];
    char recv_buf[SIZE];

    struct sockaddr_in addr = {0};
    socklen_t addr_len = sizeof(sockaddr_in);

    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &addr.sin_addr);

    size_t recv_len;


    while (true)
    {
        std::cout << "$ ";
        std::cin.getline(send_buf, SIZE);

        sendto(sock, send_buf, SIZE, 0,
               reinterpret_cast<const sockaddr *>(&addr),
               addr_len);

        recv_len = recvfrom(sock, recv_buf, SIZE, 0,
                            reinterpret_cast<sockaddr *>(&addr),
                            &addr_len);
        if( recv_len > 0 )
        {
            recv_buf[recv_len] = '\0';
            std::cout << recv_buf << std::endl;
        }
    }

    return EXIT_SUCCESS;
}