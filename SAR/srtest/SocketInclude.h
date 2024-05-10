#ifndef SOCKETINCLUE_H
#define SOCKETINCLUE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <map>
#include <netinet/tcp.h>
#include <string.h>
#include <vector>
#include <QTextStream>

#define UDP_MTU             1472
#define MAX_EPOLL_EVENTS    1024         //epoll监听最大事件数量，对应最大连接socket数量

#pragma pack(push, 1)

//业务包头
struct CommMsgHdr
{
    uint16_t uMsgType;
    uint32_t uTotalLen;
};

//分包包头
struct MergeHdr
{
    CommMsgHdr  msgHead;//公共包头
    unsigned int uSequence;//整包的序号
    unsigned int uCurPktSize;//当前包的大小(sizeof(MergeHdr)+负载长度)
    unsigned int uAllPktSize;//数据的总大小
    unsigned int uPieces;//数据被分成包的个数
    unsigned int uIndex;//数据包当前的帧号
    unsigned int uOffset;//数据包在整个数据中的偏移
};
#define UDP_PAYLOAD         ( UDP_MTU - sizeof(MergeHdr) )    //互联网udp有效负载

//单个Sequence的所有包
struct pkt_merge
{
    void* mergebuff        = nullptr ;  //合并后的数据
    int  uAllPktSize       = 0;     //接收总长度
    unsigned int uSequence = 0;     //整包的序号
    unsigned int uPieces;           //数据被分成包的个数

    std::map<unsigned int,void*> mRcvData;//暂存接收的包<index,data>
};

//udp客户端
typedef struct _UdpClientDef_{
    int32_t             fd;              //fd，如果是服务端则统一使用服务端的fd，根据地址区分不同客户端
    struct sockaddr_in  remote_addr;     //对端地址
    std::map<int,pkt_merge*> m_pkt_merge;//<sequence,pkt_merge>,需要组包时，暂存包
}UdpClientDef;

//udp服务端
typedef struct _UdpServerDef_{
    int32_t fd;                         //fd
    struct sockaddr_in local_addr;      //本端地址
}UdpServerDef;

#pragma pack(pop)

int32_t CreateUdpClient(UdpClientDef* udp, uint32_t remoteIp, int32_t remotePort);
int32_t CreateUdpServer(UdpServerDef *udp, int32_t plocalPort);

#endif // SOCKETINCLUE_H
