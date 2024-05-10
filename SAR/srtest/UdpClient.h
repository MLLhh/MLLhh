#ifndef UDPCLIENT_H
#define UDPCLIENT_H
#include<QDebug>

#include "SocketInclude.h"
class UdpClient
{
public:
    UdpClient();
    ~UdpClient();
    int CreateUdpCli(uint32_t serverIp, uint16_t _uListenPort);
    int dealUdpSendData();
private:
    UdpClientDef*    pUdpClientDef;
};
#endif //UDPCLIENT_H
