#include <iostream>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>

int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, IPPROTO_TCP};

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    const size_t SIZE = 256;
    char send_buf[SIZE];
    char recv_buf[SIZE];

    size_t recv_len;

    const int port { std::stoi(argv[2]) };
    const std::string host_name { argv[1] };
    std::cout << "Starting echo client on the address:port " << host_name << ":" << port << "...\n";

    addrinfo hints =
            {
                    .ai_flags= AI_CANONNAME,
                    .ai_family = AF_UNSPEC,
                    .ai_socktype = SOCK_STREAM,
                    .ai_protocol = IPPROTO_TCP
            };

    addrinfo* result = nullptr;

    const struct hostent* remote_host {};
    socklen_t addr_len = sizeof(sockaddr_in);

    getaddrinfo(host_name.c_str(), nullptr, &hints, &result);

    if(AF_INET == result->ai_family)
    {
        sockaddr_in addr = *reinterpret_cast<const sockaddr_in*>(result->ai_addr);
        addr.sin_family = result->ai_family;
        addr.sin_port = htons(port);

        if (connect(sock, reinterpret_cast<const sockaddr* const>(&addr), sizeof(addr)) != 0)
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if(AF_INET6 == result->ai_family)
    {
        sockaddr_in6 addr = *reinterpret_cast<const sockaddr_in6*>(result->ai_addr);
        addr.sin6_family = result->ai_family;
        addr.sin6_port = htons(port);

        if (connect(sock, reinterpret_cast<const sockaddr* const>(&addr), sizeof(addr)) != 0)
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
            return EXIT_FAILURE;
        }
    }

    freeaddrinfo(result);

    const int flag = 1;

    if (ioctl(sock, FIONBIO, const_cast<int *>(&flag)) < 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char
    *>(&flag), sizeof(flag)) < 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    while (true)
    {
        std::cout << "$ ";
        std::cin.getline(send_buf, SIZE);

        send(sock, send_buf, SIZE, 0);

        while(true)
        {
            recv_len = recv(sock, recv_buf, SIZE, 0);
            if( recv_len > 0 )
            {
                recv_buf[recv_len] = '\0';
                std::cout << "------------\n" << recv_buf << "------------\n" << std::endl;
                continue;
            }
            else if(-1 == recv_len)
            {
                if (EINTR == errno) continue;
                if (0 == errno) break;
                break;
            }
            break;
        }
    }

    return EXIT_SUCCESS;
}  
