#include "xsender.h"
#include <iostream>
#include <utils.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ether.h>

XSender::XSender(QObject *parent, int sockfd, int ifindex) : QObject(parent)
{
    std::cout << "XSender::XSender()" << std::endl;
    this->sockfd = sockfd;

    bzero(&sa, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = ETH_P_ALL;
    sa.sll_ifindex = ifindex;
    sa.sll_hatype = ARPHRD_ETHER;
    sa.sll_pkttype = PACKET_OTHERHOST;
    sa.sll_halen = 6;
}
XSender::~XSender()
{
    std::cout << "XSender::~XSender()" << std::endl;
}
void XSender::send(char *data, int size)
{
    int send_size = sendto(sockfd, data, size, 0, (sockaddr*)&sa, sizeof(struct sockaddr_ll));
    if (send_size < 0) {
        std::cout << "failed: sendto socket" << std::endl;
        std::cout << strerror(errno) << std::endl;
        std::cout << "ifindex = " << sa.sll_ifindex << std::endl;
    } else {
        Utils::printOut(data, size);
    }
}
