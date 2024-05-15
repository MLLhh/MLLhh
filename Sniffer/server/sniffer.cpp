#include "sniffer.h"

Sniffer::Sniffer(QObject *parent) : QObject(parent)
{

}
Sniffer::~Sniffer()
{

}

void Sniffer::print_hex_data(const unsigned char* data, ssize_t size) {
    for (ssize_t i = 0; i < size; ++i) {
        printf("%02x ", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void Sniffer::startSniffer()
{
        int sock_raw;
        ssize_t data_size;
        unsigned char buffer[65536];
        struct sockaddr saddr;
        socklen_t saddr_size = sizeof(saddr);

        // 创建原始套接字，可以通过以太网接口接收所有类型的数据包
        //sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
        if (sock_raw < 0) {
            perror("Socket Error");
        }

        while (1) {
            // 接收数据包
            data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, &saddr, &saddr_size);
            std::cout << QString(saddr.sa_data).toUtf8().toHex(' ').toStdString() << std::endl;
            // 打印接收到的数据包大小
            std::cout << "Received packet size: " << data_size << std::endl;
            if (data_size < 0) {
                std::cerr << "Recvfrom error, failed to get packets" << std::endl;
                break;
            }

            // 解析以太网头部
            struct ethhdr *eth = (struct ethhdr *)buffer;
            std::cout << "Source MAC: ";
            for (int i = 0; i < ETH_ALEN; i++) {
                printf("%02x:", eth->h_source[i]);
            }
            std::cout << std::endl;

            std::cout << "Destination MAC: ";
            for (int i = 0; i < ETH_ALEN; i++) {
                printf("%02x:", eth->h_dest[i]);
            }
            std::cout << std::endl;

            std::cout << "Protocol: " << ntohs(eth->h_proto) << std::endl;

            // 打印以太网数据
            std::cout << "Ethernet Data:" << std::endl;
            //print_hex_data(buffer + sizeof(struct ethhdr), data_size - sizeof(struct ethhdr));
            std::cout << std::endl;
            // 根据协议类型进行进一步处理
            // ...


        }

        close(sock_raw);

}
