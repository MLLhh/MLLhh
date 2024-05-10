#ifndef UDPSERVER_H
#define UDPSERVER_H

#include "SocketInclude.h"
using namespace std;
class UdpServer
{
public:
    UdpServer();
    ~UdpServer();
    bool StartUp(uint16_t _uListenPort);//启动服务
private:
    static void* ThreadCallBack(void *arg);//创建线程函数
    void DealUdpThread();//udp线程处理函数
    void TryMergePkt(int index, unsigned int seq);//尝试组包
    void DelMergePkt(int index, unsigned int seq);//释放包
    void SetUdpEpollFlag(int fd, bool flag);

private:
    UdpServerDef*    pUdpServerDef;    //udp服务端对象
    pthread_t        m_threadId;    //udp线程ID
    int              m_epoll_fd;    //epollfd
    bool             m_bIsRunning;  //线程是否运行
    vector<UdpClientDef> m_vUdpClientDef;//定义了一个名为 m_vUdpClientDef 的 vector 对象，它是一个模板类，用于存储 UdpClientDef 类型的对象。
};
#endif //UDPSERVER_H

