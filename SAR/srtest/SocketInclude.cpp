#include<QDebug>
#include "SocketInclude.h"

//创建udp套接字
int32_t CreateUdpClient(UdpClientDef* udp, uint32_t remoteIp, int32_t remotePort)
{
    if (udp == NULL)		return -1;
    udp->fd = -1;

    udp->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    fcntl(udp->fd, F_SETFL, O_NONBLOCK);//设置非阻塞,以便在没有数据可读时立即返回
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

int32_t CreateUdpServer(UdpServerDef *udp, int32_t plocalPort)
{
    if (udp == NULL)		return -1;
    udp->fd = -1;


    udp->local_addr.sin_family = AF_INET;//设置地址族为 AF_INET，表示 IPv4。
    udp->local_addr.sin_port = htons(plocalPort);//将 plocalPort 转换为网络字节序，并设置为本地端口号。
    udp->local_addr.sin_addr.s_addr = INADDR_ANY;//设置本地地址为 INADDR_ANY，表示接受任何接口的连接。
    udp->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);//创建一个 UDP socket
    if(udp->fd < 0)
    {
        printf("[CreateUdpServer] create udp socket failed,errno=[%d],plocalPort=[%d]",errno,plocalPort);
        return -1;
    }


    //2.socket参数设置
    int opt = 1;
    setsockopt(udp->fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//设置 socket 选项 SO_REUSEADDR，允许在同一端口上重新绑定 socket。
    fcntl(udp->fd, F_SETFL, O_NONBLOCK);//设置非阻塞,以便在没有数据可读时立即返回

    if (bind(udp->fd, (struct sockaddr*) &udp->local_addr,sizeof(struct sockaddr_in)) < 0)
    {
        close(udp->fd);
        printf("[CreateUdpServer] Udp server bind failed,errno=[%d],plocalPort=[%d]",errno,plocalPort);
        return -2;
    }

    qDebug()<<"客户端套接字创建成功"<<endl;
    return 0;
}
