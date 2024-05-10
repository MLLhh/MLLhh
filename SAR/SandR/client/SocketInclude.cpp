#include<QDebug>
#include "SocketInclude.h"

//创建udp套接字
int32_t CreateUdpClient(UdpClientDef* udp, uint32_t remoteIp, int32_t remotePort)
{
    if (udp == NULL)		return -1;
    udp->fd = -1;

    udp->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    fcntl(udp->fd, F_SETFL, O_NONBLOCK);//设置非阻塞
    if(udp->fd < 0)
    {
        printf("[CreateUdpClient] create udp socket failed,errno=[%d],remoteIp=[%u],remotePort=[%d]",errno,remoteIp,remotePort);
        return -1;
    }


    udp->remote_addr.sin_family = AF_INET;
    udp->remote_addr.sin_port = htons(remotePort);
    udp->remote_addr.sin_addr.s_addr = remoteIp;
    qDebug()<<"服务端套接字创建成功"<<endl;
    qDebug()<<"端口号"<<udp->remote_addr.sin_port<<endl;
    qDebug()<<"IP地址"<<udp->remote_addr.sin_addr.s_addr<<endl;
    return 0;
}
