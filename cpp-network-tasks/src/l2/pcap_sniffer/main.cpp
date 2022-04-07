#include <iostream>
#include <string>
#include <cctype>
#include <cerrno>

//#include <pcap.h>
#include <PcapLiveDeviceList.h>
#include <SystemUtils.h>

/**
* A struct for collecting packet statistics
*/
struct PacketStats
{
    int ethPacketCount;
    int ipv4PacketCount;
    int ipv6PacketCount;
    int tcpPacketCount;
    int udpPacketCount;
    int dnsPacketCount;
    int httpPacketCount;
    int sslPacketCount;

    /**
    * Clear all stats
    */
    void clear() { ethPacketCount = 0; ipv4PacketCount = 0; ipv6PacketCount = 0; tcpPacketCount = 0; udpPacketCount = 0; tcpPacketCount = 0; dnsPacketCount = 0; httpPacketCount = 0; sslPacketCount = 0; }

    /**
    * C'tor
    */
    PacketStats() { clear(); }

    /**
    * Collect stats from a packet
    */
    void consumePacket(pcpp::Packet& packet)
    {
        if (packet.isPacketOfType(pcpp::Ethernet))
            ethPacketCount++;
        if (packet.isPacketOfType(pcpp::IPv4))
            ipv4PacketCount++;
        if (packet.isPacketOfType(pcpp::IPv6))
            ipv6PacketCount++;
        if (packet.isPacketOfType(pcpp::TCP))
            tcpPacketCount++;
        if (packet.isPacketOfType(pcpp::UDP))
            udpPacketCount++;
        if (packet.isPacketOfType(pcpp::DNS))
            dnsPacketCount++;
        if (packet.isPacketOfType(pcpp::HTTP))
            httpPacketCount++;
        if (packet.isPacketOfType(pcpp::SSL))
            sslPacketCount++;
    }

    /**
    * Print stats to console
    */
    void printToConsole()
    {
        std::cout
                << "Ethernet packet count: " << ethPacketCount << std::endl
                << "IPv4 packet count:     " << ipv4PacketCount << std::endl
                << "IPv6 packet count:     " << ipv6PacketCount << std::endl
                << "TCP packet count:      " << tcpPacketCount << std::endl
                << "UDP packet count:      " << udpPacketCount << std::endl
                << "DNS packet count:      " << dnsPacketCount << std::endl
                << "HTTP packet count:     " << httpPacketCount << std::endl
                << "SSL packet count:      " << sslPacketCount << std::endl;
    }
};

int main(int argc, const char * const argv[])
{
    std::string interfaceAddress;
    // Check for capture device name on command-line.
    if (2 == argc)
    {
        interfaceAddress = argv[1];
    }
    else
    {
        std::cerr << "error: unrecognized command-line options\n" << std::endl;
        std::cout
            << "Usage: " << argv[0] << " [interface]\n\n"
            << "Options:\n"
            << "    interface    Listen on <interface> for packets.\n"
            << std::endl;

        exit(EXIT_FAILURE);
    }

    // find the interface by IP address
    pcpp::PcapLiveDevice* dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(interfaceAddress);
    if (dev == NULL)
    {
        std::cerr << "Cannot find interface with IPv4 address of '" << interfaceAddress << "'" << std::endl;
        return 1;
    }

    // before capturing packets let's print some info about this interface
    std::cout
            << "Interface info:" << std::endl
            << "   Interface name:        " << dev->getName() << std::endl // get interface name
            << "   Interface description: " << dev->getDesc() << std::endl // get interface description
            << "   MAC address:           " << dev->getMacAddress() << std::endl // get interface MAC address
            << "   Default gateway:       " << dev->getDefaultGateway() << std::endl // get default gateway
            << "   Interface MTU:         " << dev->getMtu() << std::endl; // get interface MTU

    if (dev->getDnsServers().size() > 0)
        std::cout << "   DNS server:            " << dev->getDnsServers().at(0) << std::endl;

    // open the device before start capturing/sending packets
    if (!dev->open())
    {
        std::cerr << "Cannot open device" << std::endl;
        return 1;
    }

    // create the stats object
    PacketStats stats;

    std::cout << std::endl << "Starting capture with packet vector..." << std::endl;

// create an empty packet vector object
    pcpp::RawPacketVector packetVec;

// start capturing packets. All packets will be added to the packet vector
    dev->startCapture(packetVec);

    // sleep for 10 seconds in main thread, in the meantime packets are captured in the async thread
    pcpp::multiPlatformSleep(10);

// stop capturing packets
    dev->stopCapture();

    // go over the packet vector and feed all packets to the stats object
    for (pcpp::RawPacketVector::ConstVectorIterator iter = packetVec.begin(); iter != packetVec.end(); iter++)
    {
        // parse raw packet
        pcpp::Packet parsedPacket(*iter);

        // feed packet to the stats object
        stats.consumePacket(parsedPacket);
    }

    // print results
    std::cout << "Results:" << std::endl;
    stats.printToConsole();

    return EXIT_SUCCESS;
}
