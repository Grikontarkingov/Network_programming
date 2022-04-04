#include <iostream>
#include <netinet/ip_icmp.h>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


int main(int argc, char *const argv[]) {
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string &host_name {argv[1]};

    socket_wrapper::Socket sock = {AF_INET, SOCK_RAW, IPPROTO_ICMP};

    addrinfo hints =
            {
                .ai_flags = AI_CANONNAME,
                .ai_family = AF_UNSPEC,
                .ai_socktype = SOCK_STREAM,
                .ai_protocol = 0
            };

    addrinfo *servinfo = nullptr;
    int status = 0;
    if ((status = getaddrinfo(host_name.c_str(), nullptr, &hints, &servinfo)) != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return EXIT_FAILURE;
    }

    if (AF_INET == servinfo->ai_family)
    {
        const sockaddr_in* const sin = reinterpret_cast<const sockaddr_in* const>(servinfo->ai_addr);
    }
    else if (AF_INET6 == servinfo->ai_family)
    {
        const sockaddr_in6* const sin = reinterpret_cast<const sockaddr_in6* const>(servinfo->ai_addr);
    }

    struct timeval* tv;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*> (&tv), sizeof(tv));

    uint16_t num_of_cycles = 4;

    for(uint16_t i = 0; i < num_of_cycles; ++i)
    {
        struct icmphdr* icmp_package;
        pid_t pid = getpid();

        /*icmp_package->type = ICMP_ECHO;
        icmp_package->code = 0;
        icmp_package->un.echo.id = pid;
        icmp_package->checksum = ;
        icmp_package->un.echo. = "xxx";*/

    }

    freeaddrinfo(servinfo);
    return 0;
}
