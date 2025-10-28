#include "xlistener.h"
#include <sys/socket.h>
#include <iostream>
#include <utils.h>
#include <xglobal.h>
#include <csignal>

XListener::XListener(QObject *parent, int sockfd) : QObject(parent)
{
    std::cout << "XReceiver::XReceiver()" << std::endl;
    this->sockfd = sockfd;

    signal(SIGUSR1, XListener::staticSignalHandler); // 注册信号处理函数,处理通知退出循环体的中断信号
}
XListener::~XListener()
{
    std::cout << "XReceiver::~XReceiver()" << std::endl;
}
void XListener::loop()
{
    flag_xreceiver_loop = true;
    while (flag_xreceiver_loop) {
        int size = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (size < 0) {
            std::cout << "failed: recvfrom socket" << std::endl;
            std::cout << strerror(errno) << std::endl;
        } else {
            Utils::printIn(buffer, size);
            emit sigRxData(buffer, size);
//            index++;
        }
    }
}
/* 信号处理函数，收到中断信号，退出循环体 */
void XListener::staticSignalHandler(int)
{
    flag_xreceiver_loop = false;
}
