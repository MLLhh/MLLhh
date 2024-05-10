#include "udpserver.h"
#include <string>
#include <sys/epoll.h>
#include <QDebug>

UdpServer::UdpServer()
{
    pUdpServerDef = new UdpServerDef;
    m_epoll_fd = epoll_create(1);
    m_bIsRunning = true;
    qDebug()<<"构造函数的m_bIsRunning"<<m_bIsRunning<<endl;
}

UdpServer::~UdpServer()
{
    m_bIsRunning = false;
    qDebug()<<"析构函数的m_bIsRunning"<<m_bIsRunning<<endl;
}

bool UdpServer::StartUp(uint16_t _uListenPort)
{
    if(CreateUdpServer(pUdpServerDef,_uListenPort) == 0)
    {
        qDebug()<<"开始事件监听"<<m_bIsRunning<<endl;
        SetUdpEpollFlag(pUdpServerDef->fd,true);
    }


    qDebug()<<"StartUp的m_bIsRunning"<<m_bIsRunning<<endl;
    if(pthread_create(&m_threadId, nullptr, ThreadCallBack, this))
    {
        printf("[UdpServer::StartUp] create thread failed.");
        return false;
    }

    return true;
}

//TCP服务监听线程，处理接入监听，客户端断开/错误管理
void* UdpServer::ThreadCallBack(void *arg)
{

    UdpServer *tm = static_cast<UdpServer *>(arg);
    if(tm)
        tm->DealUdpThread();
    return nullptr;
/*
if(tm)：检查转换后的指针 tm 是否有效（即不为 nullptr）。
*/
}

void UdpServer::DealUdpThread()
{
    qDebug()<<"线程创建成功"<<endl;
    const int kEpollDefaultWait = 1;//超时时长，单位ms
    struct epoll_event alive_events[MAX_EPOLL_EVENTS];

    void* bigBuffer = NULL;
    int  mergeRcvLen = 0;
    static unsigned int sequence = 1;
    qDebug()<<"线程的m_bIsRunning"<<m_bIsRunning<<endl;
    while (m_bIsRunning)
    {
//qDebug()<<"进入事件驱动循环"<<endl;
        int num = epoll_wait(m_epoll_fd, alive_events, MAX_EPOLL_EVENTS, kEpollDefaultWait);
//qDebug()<<"num="<<num<<endl;
        for (int i = 0; i < num; ++i)
        {
            int fd = alive_events[i].data.fd;
            int events = alive_events[i].events;

//qDebug()<<"1！"<<endl;
            if ( events & EPOLLIN )
            {
                char recv_buffer[UDP_MTU];
                memset(recv_buffer,0,UDP_MTU);
                ssize_t recv_len = 0;
                socklen_t src_len = sizeof(struct sockaddr_in);
                struct sockaddr_in SrcAddr;
                memset(&SrcAddr, 0, src_len);

//qDebug()<<"2！"<<endl;
                //1.开始接收
                struct sockaddr_in remote_addr;
                if ((recv_len = recvfrom(fd, recv_buffer, UDP_MTU, 0,	(struct sockaddr*) &SrcAddr, &src_len)) > 0)
                {
                    qDebug()<<"收到数据！"<<endl;
                    m_bIsRunning=false;
                    remote_addr.sin_port = SrcAddr.sin_port;
                    remote_addr.sin_addr.s_addr = SrcAddr.sin_addr.s_addr;

                    //判断是否已记录该客户端
                    bool isExist = false;
                    for(int index=0;index<m_vUdpClientDef.size();index++)
                    {
                        if(m_vUdpClientDef[index].remote_addr.sin_addr.s_addr == remote_addr.sin_addr.s_addr
                        && m_vUdpClientDef[index].remote_addr.sin_port        == remote_addr.sin_port)
                        {
                            qDebug()<<"客户端已记录！"<<endl;
                            isExist = true;
                            break;
                        }
                    }
                    if(!isExist)
                    {
                        qDebug()<<"客户端未记录！"<<endl;
                        UdpClientDef tUdpClientDef;
                        tUdpClientDef.fd = fd;
                        tUdpClientDef.remote_addr = remote_addr;
                        m_vUdpClientDef.push_back(tUdpClientDef);
                    }
                }
                else
                    continue;

                //1.不需要组包
                if( ((CommMsgHdr *)recv_buffer)->uMsgType != 635)
                {
                    qDebug()<<"小于MTU！"<<endl;
                    //直接分发处理
                }

                //2.需要组包，处理错序
                for(int index=0;index<m_vUdpClientDef.size();index++)
                {
                    if(m_vUdpClientDef[index].remote_addr.sin_addr.s_addr == remote_addr.sin_addr.s_addr
                    && m_vUdpClientDef[index].remote_addr.sin_port        == remote_addr.sin_port)
                    {
                        MergeHdr* tMergeHdr = (MergeHdr*)recv_buffer;

                        if(m_vUdpClientDef[index].m_pkt_merge.count(tMergeHdr->uSequence) > 0)
                        {
                            //已存在该Sequence的包
                            qDebug()<<"已存在的包！"<<endl;
                            m_vUdpClientDef[index].m_pkt_merge[tMergeHdr->uSequence]->uAllPktSize += tMergeHdr->uCurPktSize - sizeof(MergeHdr);;
                        }
                        else
                        {
                            //新Sequence的包
                            //建议新增定时器，在一定时间内没有收完包，则丢弃包
                            qDebug()<<"未存在的包！"<<endl;
                            pkt_merge *tpkt_merge = new pkt_merge;
                            tpkt_merge->uSequence = tMergeHdr->uSequence;
                            tpkt_merge->uAllPktSize = tMergeHdr->uCurPktSize - sizeof(MergeHdr);
                            tpkt_merge->uPieces = tMergeHdr->uPieces;
                            tpkt_merge->mergebuff = new char[tMergeHdr->uAllPktSize];

                            m_vUdpClientDef[index].m_pkt_merge[tMergeHdr->uSequence] = tpkt_merge;
                        }
                        //todo,如果已存在key,存在泄漏
                        void* buff = new char[UDP_MTU];
                        memcpy(buff,recv_buffer,UDP_MTU);
                        m_vUdpClientDef[index].m_pkt_merge[tMergeHdr->uSequence]->mRcvData[tMergeHdr->uIndex] = buff;

                        qDebug()<<"将数据放入临时缓存！"<<endl;
                        TryMergePkt(index,tMergeHdr->uSequence);
                    }
                }

#if 0         //3.需要组包，不处理错序,此时如果同时接收两个客户端的数据，则可能出现错序
              if( ((CommMsgHdr *)recv_buffer)->uMsgType == 635)
                {
                    MergeHdr* tMergeHdr = (MergeHdr*)recv_buffer;
                    //根据sequence按顺序接收
                    if(sequence == tMergeHdr->uIndex)
                    {
                        sequence++;
                        if(bigBuffer == nullptr)
                            bigBuffer = new char[tMergeHdr->uAllPktSize];
                        mergeRcvLen += tMergeHdr->uCurPktSize - sizeof(MergeHdr);

                        memcpy((char*)bigBuffer+tMergeHdr->uOffset, recv_buffer + sizeof(MergeHdr),
                               tMergeHdr->uCurPktSize - sizeof(MergeHdr));
                        if ((tMergeHdr->uPieces == tMergeHdr->uIndex)
                                && (mergeRcvLen == tMergeHdr->uAllPktSize))
                        {
                            //组包完成
                            CommMsgHdr* pMsg = (CommMsgHdr*)bigBuffer;
                            printf("pMsg.uMsgType=%d\n",pMsg->uMsgType);
                            printf("pMsg.uTotalLen=%d\n",pMsg->uTotalLen);
                            printf("remote_addr.sin_port=%d\n",ntohs(remote_addr.sin_port));

                            mergeRcvLen = 0;
                            delete[] (char *)(bigBuffer);
                            bigBuffer = NULL;
                            sequence = 1;
                        }
                    }
                    //如果出现错序或乱序，丢弃包
                    else
                    {
                        mergeRcvLen = 0;
                        delete[] (char *)(bigBuffer);
                        bigBuffer = NULL;
                        sequence = 1;
                        printf(" miss-sequence \n");
                    }
                }
#endif
            }
        }

    }
}

//尝试组包
void UdpServer::TryMergePkt(int index, unsigned int seq)
{
    qDebug()<<"TryMergePkt！"<<endl;
    pkt_merge *tpkt_merge = m_vUdpClientDef[index].m_pkt_merge[seq];
    if(tpkt_merge->uPieces > tpkt_merge->mRcvData.size())
    {
        //还没收完所有的包，不组包
        qDebug()<<"还没收完所有的包，不组包！"<<endl;
        auto last_ite = tpkt_merge->mRcvData.end();
        last_ite --;
        //如果此时收到的 分包序列号 >= 包的总数，则说明中间有丢包，此时丢弃包
        if(last_ite->first >= tpkt_merge->uPieces)
        {
            DelMergePkt(index, seq);
            printf("error,miss sequence\n");
        }
    }
    else if(tpkt_merge->uPieces < tpkt_merge->mRcvData.size())
    {
        //收到包的数量大于分片数量，出现异常，丢弃包
        printf("error,recv too many bags\n");
        DelMergePkt(index, seq);
    }
    else
    {
        //已经收完所有分包，开始组包
        qDebug()<<"已经收完所有分包，开始组包！"<<endl;
        auto ite = tpkt_merge->mRcvData.begin();
        while(ite != tpkt_merge->mRcvData.end())
        {
            MergeHdr* tMergeHdr = (MergeHdr*)ite->second;
            memcpy((char*)tpkt_merge->mergebuff+tMergeHdr->uOffset, (char*)ite->second + sizeof(MergeHdr),
                   tMergeHdr->uCurPktSize - sizeof(MergeHdr));
            ++ ite;
        }

        ite --;
        MergeHdr* tMergeHdr = (MergeHdr*)ite->second;
        if ((tpkt_merge->uPieces == tMergeHdr->uIndex)
                && (tpkt_merge->uAllPktSize == tMergeHdr->uAllPktSize))
        {
            //组包完成，分发处理
            qDebug()<<"组包完成！"<<endl;
            CommMsgHdr* pMsg = (CommMsgHdr*)tpkt_merge->mergebuff;
            printf("pMsg.uMsgType=%d\n",pMsg->uMsgType);
            printf("pMsg.uTotalLen=%d\n",pMsg->uTotalLen);

            //释放资源
            DelMergePkt(index, seq);
        }
    }
}

void UdpServer::DelMergePkt(int index, unsigned int seq)
{
    pkt_merge *tpkt_merge = m_vUdpClientDef[index].m_pkt_merge[seq];

    auto ite_2 = tpkt_merge->mRcvData.begin();
    while(ite_2 != tpkt_merge->mRcvData.end())
    {
        delete[] (char *)(ite_2->second);
        ite_2++;
    }
    delete[] (char *)(tpkt_merge->mergebuff);
    tpkt_merge->mRcvData.clear();
    m_vUdpClientDef[index].m_pkt_merge.erase(m_vUdpClientDef[index].m_pkt_merge.find(tpkt_merge->uSequence));
}

//设置epoll监听udp套接字，只监听EPOLLIN事件
void UdpServer::SetUdpEpollFlag(int fd, bool flag)
{
    qDebug()<<"正在事件监听"<<m_bIsRunning<<endl;
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;
    if(flag)
        epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,fd,&evt);
    else
        epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,fd,&evt);
}

