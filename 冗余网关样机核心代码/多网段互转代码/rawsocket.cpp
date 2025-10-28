#include "rawsocket.h"
#include <QThread>
#include <csignal>
#include <netinet/ether.h>
#include <iostream>
#include <utils.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <xsender.h>
#include <xlistener.h>
#include <unistd.h>

RawSocket::RawSocket(QObject *parent, const char* interface1) : QObject(parent)
{
    std::cout << "RawSocket::RawSocket()" << std::endl;

    initSocket(sockfd1, interface1, ifindex1);
//    initSocket(sockfd2, interface2, ifindex2);

    start();
}
RawSocket::~RawSocket()
{
    std::cout << "RawSocket::~RawSocket()" << std::endl;

    stop();
}

// 初始化socket
void RawSocket::initSocket(int &sockfd, const char *interface, int &ifindex)
{
    std::cout << "RawSocket::initSocket()" << interface << std::endl;

    // 创建socket
    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        std::cout << "failed: create sockfd" << std::endl;
        std::cout << strerror(errno) << std::endl;
        return;
    } else {
        std::cout << "ok: create sockfd" << std::endl;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_ifrn.ifrn_name, interface, IFNAMSIZ); // 选中网卡
    int ret;

    // 开启网卡混杂模式
    ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr); // 读取标志位
    if (ret == -1) {
        std::cout << "failed: ioctl get flags" << std::endl;
        close(sockfd);
        return;
    } else {
        std::cout << "ok: ioctl get flags" << std::endl;
    }
    ifr.ifr_ifru.ifru_flags |= IFF_PROMISC; // 修改标志位:开启混杂模式
//    ifr.ifr_ifru.ifru_flags &= ~IFF_PROMISC; // 修改标志位:关闭混杂模式
    ret = ioctl(sockfd, SIOCSIFFLAGS, &ifr); // 写入标志位
    if (ret == -1) {
        std::cout << "failed: ioctl set flags" << std::endl;
        close(sockfd);
        return;
    } else {
        std::cout << "ok: ioctl set flags" << std::endl;
    }

    // 设置并绑定socket到网卡
    ifindex = if_nametoindex(interface);
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = ifindex;
    ret = ::bind(sockfd, (struct sockaddr*)&sll, sizeof(struct sockaddr_ll));
    if (ret == -1) {
        std::cout << "failed: bind socket to interface" << std::endl;
        std::cout << strerror(errno) << std::endl;
        close(sockfd);
        return;
    } else {
        std::cout << "ok: bind to interface " << interface << std::endl;
    }
}
// 解析数据包
void RawSocket::analyze(char *data, int size)
{
    std::cout << std::endl << "[RX] size = " << size << std::endl;
    ethhdr *pEth = (ethhdr*)data;
    std::cout << "dst = ";
    Utils::printHex(pEth->h_dest, ETH_ALEN);
    std::cout << "src = ";
    Utils::printHex(pEth->h_source, ETH_ALEN);
    ushort protocol = ntohs(pEth->h_proto);
    std::cout << "protocol = " << protocol;
    switch (protocol) {
    case ETH_P_IP:
        std::cout << " [IPv4]";
        break;
    case ETH_P_IPV6:
        std::cout << " [IPv6]";
        break;
    case ETH_P_ARP:
        std::cout << " [ARP]";
        break;
    default:
        std::cout << " [UNKNOWN]";
        break;
    }
    std::cout << std::endl;
    Utils::printHex(data, size);
}
void RawSocket::start()
{
    // sockfd1/interface1 >>> sockfd2/interface2
    XListener *listener1 = new XListener(nullptr, sockfd1);
    rxThread1 = new QThread(this);
    listener1->moveToThread(rxThread1);
    rxThread1->start(QThread::Priority::TimeCriticalPriority);

    XSender *sender1 = new XSender(nullptr, sockfd1, ifindex1);
    txThread1 = new QThread(this);
    sender1->moveToThread(txThread1);
    txThread1->start(QThread::Priority::TimeCriticalPriority);

//    // sockfd2/interface2 >>> sockfd1/interface1
//    XListener *listener2 = new XListener(nullptr, sockfd2);
//    rxThread2 = new QThread(this);
//    listener2->moveToThread(rxThread2);
//    rxThread2->start(QThread::Priority::TimeCriticalPriority);

//    XSender *sender2 = new XSender(nullptr, sockfd1, ifindex1);
//    txThread2 = new QThread(this);
//    sender2->moveToThread(txThread2);
//    txThread2->start(QThread::Priority::TimeCriticalPriority);

    // bind signals and slots
    connect(this, &RawSocket::sigStart, listener1, &XListener::loop);
//    connect(this, &RawSocket::sigStart, listener2, &XListener::loop);

    // 自动双向转发
//    connect(listener1, &XListener::sigRxData, sender1, &XSender::send);
//    connect(listener2, &XListener::sigRxData, sender2, &XSender::send);
    connect(listener1, &XListener::sigRxData, this, &RawSocket::read);
    connect(this, &RawSocket::sigSend, sender1, &XSender::send);

    // send start signal
    emit sigStart();
}
void RawSocket::stop()
{
    std::cout << "RawSocket::stop()" << std::endl;
    std::raise(SIGUSR1); // 使用中断信号终止XReceiver的循环体

    rxThread1->quit();
    txThread1->quit();
    rxThread2->quit();
    txThread2->quit();

//    rxThread1->wait();
//    txThread1->wait();
//    rxThread2->wait();
//    txThread2->wait();

    close(sockfd1);
//    close(sockfd2);
    sockfd1 = -1;
//    sockfd2 = -1;
}
void RawSocket::read(char *data, int size)
{
    emit sigRead(data, size);
}
void RawSocket::send(char *data, int size)
{
    emit sigSend(data, size);
}
