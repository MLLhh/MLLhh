#include "UdpServer.h"
#include <string>
#include <sys/epoll.h>
#include <QDebug>

UdpServer::UdpServer()
{
    pUdpServerDef = new UdpServerDef;//SocketInclude.h的结构体,

    m_epoll_fd = epoll_create(1);
   /*epoll_create(1) 是一个系统调用，它用于创建一个新的 epoll 实例
     epoll 是 Linux 内核提供的一种 I/O 事件通知机制，它可以高效地等待多个文件描述符（fd）上的 I/O 事件
   */

    m_bIsRunning = true;//线程是否运行的标志
    qDebug()<<"构造函数的m_bIsRunning"<<m_bIsRunning<<endl;
}

UdpServer::~UdpServer()
{
    m_bIsRunning = false;
    qDebug()<<"析构函数的m_bIsRunning"<<m_bIsRunning<<endl;
}

bool UdpServer::StartUp(uint16_t _uListenPort)
{
    if(CreateUdpServer(pUdpServerDef,_uListenPort) == 0)//创建socket服务端连接
    {
        SetUdpEpollFlag(pUdpServerDef->fd,true);//设置epoll监听udp套接字
    }


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
    /*
    if(tm)：检查转换后的指针 tm 是否有效（即不为 nullptr）。
    */
    UdpServer *tm = static_cast<UdpServer *>(arg);
    if(tm)
        tm->DealUdpThread();
    return nullptr;
}

void UdpServer::DealUdpThread()
{
    qDebug()<<"线程创建成功"<<endl;
    const int kEpollDefaultWait = 1;//事件监听超时时长，单位ms

    struct epoll_event alive_events[MAX_EPOLL_EVENTS];
/*
    epoll_event有events和data两个成员变量
    events 是一个位掩码，用于指示发生的事件类型。
    data 是一个联合体 epoll_data_t，它可以包含一个文件描述符data.fd、一个指向用户数据的指针ptr
*/

    void* bigBuffer = NULL;
    int  mergeRcvLen = 0;
    static unsigned int sequence = 1;
    qDebug()<<m_bIsRunning<<endl;//测试程序是否执行到这里,这个文件里的所有qdebug都是为了测试程序走到哪里
    while (m_bIsRunning)//当线程在运行
    {
        qDebug()<<"进入事件驱动循环"<<endl;
        int num = epoll_wait(m_epoll_fd, alive_events, MAX_EPOLL_EVENTS, kEpollDefaultWait);
/*
        m_epoll_fd: 这是一个 epoll 实例的文件描述符，它由 epoll_create() 创建。
        alive_events: 这是一个指向 struct epoll_event 结构体数组的指针，用于存储发生的事件。
        MAX_EPOLL_EVENTS: 这是 alive_events 数组的最大大小，它定义了可以存储的事件数量。
        kEpollDefaultWait: 这是一个超时值，以毫秒为单位。此函数kEpollDefaultWait = 1;
        epoll_wait() 函数会阻塞直到有事件发生或超时。如果在超时时间内有事件发生，函数会返回发生的事件数量。如果没有事件发生，并且超时时间已经过去，函数会返回 0。如果函数被信号中断，它会返回 -1，并设置 errno 为 EINTR。
*/

        for (int i = 0; i < num; ++i)
        {
            int fd = alive_events[i].data.fd;
            int events = alive_events[i].events;

            //如果有事件发生并且可以读取数据
            if ( events & EPOLLIN )
            {
                char recv_buffer[UDP_MTU];
                memset(recv_buffer,0,UDP_MTU);//缓冲区置零,存放数据
                ssize_t recv_len = 0;
                socklen_t src_len = sizeof(struct sockaddr_in);
                struct sockaddr_in SrcAddr;
                memset(&SrcAddr, 0, src_len);//缓冲区置零,存放源地址

                //1.开始接收
                struct sockaddr_in remote_addr;
                if ((recv_len = recvfrom(fd, recv_buffer, UDP_MTU, 0,	(struct sockaddr*) &SrcAddr, &src_len)) > 0)
                {
                    qDebug()<<"收到数据！"<<endl;
                    remote_addr.sin_port = SrcAddr.sin_port;
                    remote_addr.sin_addr.s_addr = SrcAddr.sin_addr.s_addr;

                    bool isExist = false;
                  //判断是否已记录该客户端
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
                      //push_back 是 std::vector 的一个成员函数，用于在向量（vector）的末尾添加一个新的元素。
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

                //m_vUdpClientDef是UdpServer.h中的模板类:vector<UdpClientDef> m_vUdpClientDef;

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
                            //这里只是记录一下size，并无实际的组包操作,作用是后续比对组包是否完整
                        }
                        else
                        {
                            //新Sequence的包
                            //建议新增定时器，在一定时间内没有收完包，则丢弃包
                            qDebug()<<"未存在的包！"<<endl;
                            pkt_merge *tpkt_merge = new pkt_merge;//SocketInclude.h中的结构体，记录了包的相关信息
                            tpkt_merge->uSequence = tMergeHdr->uSequence;//记录包的序号
                            tpkt_merge->uAllPktSize = tMergeHdr->uCurPktSize - sizeof(MergeHdr);//记录有效数据大小
                            tpkt_merge->uPieces = tMergeHdr->uPieces;//记录包分了几块
                            tpkt_merge->mergebuff = new char[tMergeHdr->uAllPktSize];//创建(整包-包头)大小的缓冲区

                            m_vUdpClientDef[index].m_pkt_merge[tMergeHdr->uSequence] = tpkt_merge;
                        }
                        //todo,如果已存在key,存在泄漏
                        void* buff = new char[UDP_MTU];
                        memcpy(buff,recv_buffer,UDP_MTU);//将每次收到的数据放入缓冲区
                        m_vUdpClientDef[index].m_pkt_merge[tMergeHdr->uSequence]->mRcvData[tMergeHdr->uIndex] = buff;

                        qDebug()<<"将数据放入临时缓存！"<<endl;
                        TryMergePkt(index,tMergeHdr->uSequence);//进入组包环节,程序在下面
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
    struct epoll_event evt;
    evt.events = EPOLLIN;//EPOLLIN 是一个事件标志，表示对应的文件描述符可以读取数据。
    evt.data.fd = fd;
    if(flag)
        epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,fd,&evt);
    else
        epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,fd,&evt);
}

