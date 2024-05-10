#include "UdpClient.h"
#include <string>

UdpClient::UdpClient()
{
    pUdpClientDef = new UdpClientDef;
}

UdpClient::~UdpClient()
{
    delete pUdpClientDef;
}

//本端是客户端，udp客户端建链
int UdpClient::CreateUdpCli(uint32_t serverIp,uint16_t _uListenPort)
{
    return CreateUdpClient(pUdpClientDef,serverIp,_uListenPort);
}

//处理udp数据发送
int UdpClient::dealUdpSendData()
{
    static unsigned int sequence_whole = 0;//整包的序号
    //原始数据包
    uint32_t dataLength = 1600;
    void *sendbuff = new char[dataLength];
    CommMsgHdr* pHead = (CommMsgHdr*)sendbuff;

    pHead->uMsgType = 631;
    pHead->uTotalLen = dataLength;

    //开始分包
    int packetNum = dataLength / UDP_PAYLOAD;//分包数量
    int lastPaketSize = dataLength % UDP_PAYLOAD;//最后一片包的大小
    int sequence = 0;//当前发送的包序号
    if (lastPaketSize != 0)
    {
        packetNum = packetNum + 1;
    }

    //分包的包头
    MergeHdr tMergeHdr;
    tMergeHdr.uAllPktSize = dataLength;//负载总大小
    tMergeHdr.uPieces = packetNum;//分包数量
    tMergeHdr.uSequence = sequence_whole++;
    tMergeHdr.msgHead.uMsgType = 635;

    unsigned char piecebuff[UDP_PAYLOAD + sizeof(MergeHdr)];
    while (sequence < packetNum)
    {
        memset(piecebuff,0,UDP_PAYLOAD + sizeof(MergeHdr));
        if (sequence < (packetNum-1))//不是最后一片
        {
            tMergeHdr.uCurPktSize = sizeof(MergeHdr) + UDP_PAYLOAD;//当前包大小
            tMergeHdr.uIndex = sequence + 1;//当前包序号
            tMergeHdr.uOffset = sequence * UDP_PAYLOAD;//当前包偏移
            tMergeHdr.msgHead.uTotalLen  = tMergeHdr.uCurPktSize;
            memcpy(piecebuff, &tMergeHdr, sizeof(MergeHdr));
            memcpy(piecebuff+sizeof(MergeHdr), (char*)sendbuff+tMergeHdr.uOffset, UDP_PAYLOAD);

            ssize_t send_len = ::sendto(pUdpClientDef->fd, (const char*)piecebuff, tMergeHdr.uCurPktSize, 0, (struct sockaddr*) &pUdpClientDef->remote_addr,sizeof(struct sockaddr));
            if(send_len!=tMergeHdr.uCurPktSize)
            {
                printf("udp send failed,errno=[%d]\n",errno);
            }

            qDebug()<<"send成功"<<endl;
            sequence ++;
        }
        else//最后一片
        {
            tMergeHdr.uCurPktSize = sizeof(MergeHdr)+(dataLength - sequence * UDP_PAYLOAD);
            tMergeHdr.uIndex = sequence + 1;
            tMergeHdr.uOffset = sequence * UDP_PAYLOAD;
            tMergeHdr.msgHead.uTotalLen  = tMergeHdr.uCurPktSize;
            memcpy(piecebuff, &tMergeHdr, sizeof(MergeHdr));
            memcpy(piecebuff+sizeof(MergeHdr), (char*)sendbuff+tMergeHdr.uOffset, dataLength - sequence*UDP_PAYLOAD);
            ssize_t send_len = ::sendto(pUdpClientDef->fd, (const char*)piecebuff, tMergeHdr.uCurPktSize, 0, (struct sockaddr*) &pUdpClientDef->remote_addr,sizeof(struct sockaddr));
            if(send_len!=tMergeHdr.uCurPktSize)
            {
                printf("udp send failed,errno=[%d]\n",errno);
            }

            qDebug()<<"send成功"<<endl;
            sequence ++;
        }
    }
    delete[] (char *)(sendbuff), sendbuff = NULL;
    return 0;
}

