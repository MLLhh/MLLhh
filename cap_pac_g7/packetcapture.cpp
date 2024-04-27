#include<packetcapture.h>
#include<mainwindow.h>
pcap_t *handle;


//const u_char* g_pPktdata;//捕获的数据包
//QString PacketCapture::msStrPackLen = "";//数据包的长度


//从主窗口设置网卡等信息
void PacketCapture::set_private_var(const QString &device, const QString &IpAddress, const QString &Port) {
    dev = device;
    ipAddress = IpAddress;
    port = Port;
}
//显示网卡等信息
int PacketCapture::get_private_var() {
    qDebug() << "device" << dev;
    qDebug() << "IpAddress" << ipAddress;
    qDebug() << "Port" << port;
}
//获取主窗口点击显示信息按钮得到的路径,备用:打开相关路径下的文件
void PacketCapture::setFilePath(const QString &Address)
{
    filePath = Address;
}
//类内部用于接收回调函数内的发送信号的槽函数
void PacketCapture::slot_emitsignal_cap(QString msStrPackLen,const u_char* g_pPktdata)
{

    emit extern_emitsignal_cap(msStrPackLen,g_pPktdata);//把接收到的信号再发送给外部类,一个信息中转站:中转静态函数内的数据
}
/*
//回调函数：用于操作捕获到的数据包,user用于接收外部信息,h是一个包含数据包头部信息及包长度的结构体,bytes代表捕获的数据包
void PacketCapture::packetHandler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    struct sockaddr_in *src_ip = (struct sockaddr_in *)user;
    struct ethhdr *eth = (struct ethhdr *)bytes;
    struct iphdr *ip = (struct iphdr *)(bytes + sizeof(struct ethhdr));

    struct sockaddr_in dst_ip;
    dst_ip.sin_addr.s_addr = ip->daddr;


    g_pPktdata = bytes;
    msStrPackLen = QString::number(h->len);
    if (src_ip->sin_addr.s_addr == dst_ip.sin_addr.s_addr) {
        qDebug() << "Ethernet Header:";
//输出包头部的目标IP地址
        qDebug() << "   |-Destination Address :" << QString("%1-%2-%3-%4-%5-%6")
                    .arg(eth->h_dest[0], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[1], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[2], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[3], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[4], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[5], 2, 16, QLatin1Char('0'));
//输出包头部的源IP地址
        qDebug() << "   |-Source Address      :" << QString("%1-%2-%3-%4-%5-%6")
                    .arg(eth->h_source[0], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[1], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[2], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[3], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[4], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[5], 2, 16, QLatin1Char('0'));
        qDebug() << "   |-Protocol            :" << ntohs(eth->h_proto);

    }

    // 打印整个数据包内容
    qDebug() << "Packet Dump:";
    for (int i = 0; i < h->caplen; i++) {
        if (i % 16 == 0) { // 每16个字节换行
            qDebug().noquote() << endl << QString("%1: ").arg(i, 4, 16, QChar('0')); // 打印当前偏移
        }
        qDebug().noquote() << QString("%1 ").arg(bytes[i], 2, 16, QChar('0')); // 打印每个字节的十六进制值

    }
    qDebug().noquote() << endl; // 确保在数据包末尾有一个换行


}
*/
void PacketCapture::run(){


    emit test(1);
    char errbuf[PCAP_ERRBUF_SIZE];
    get_private_var();//从主窗口设置网卡等信息
    handle = pcap_open_live(dev.toStdString().c_str(), BUFSIZ1, 1, 1000, errbuf);//打开设备创建句柄
    if (handle == NULL) {
        qDebug() << "Couldn't open device" << dev << ":" << errbuf;
        return;
    }

    struct sockaddr_in src_ip;//传递给回调函数的外部信息，主要作用是回调函数内部筛选符合指定IP地址的数据包
    inet_aton(ipAddress.toStdString().c_str(), &src_ip.sin_addr);//IP地址传递



    struct bpf_program fp;//过滤规则编译函数指针
    bpf_u_int32 maskp, netp;//表示子网掩码和IP地址
    //根据网卡、子网掩码和IP地址探测设备
    if (pcap_lookupnet(dev.toStdString().c_str(), &netp, &maskp, errbuf) == -1) {
        qDebug() << "Can't get netmask for device" << dev;
        netp = 0;
        maskp = 0;
    }
//设置过滤规则,当前按源IP和源port过滤,还可以dst ipAddress and (dst port X or X or X)
// QString filterExp = QString("src host %1 and src port %2").arg(ipAddress).arg(port);
    QString filterExp = QString("ip host %1").arg(ipAddress);

    if ((pcap_compile(handle, &fp, filterExp.toStdString().c_str(), 0, netp) == -1)){

        qDebug() << "Couldn't parse filter" << filterExp << ":" << pcap_geterr(handle);
        pcap_close(handle);
        return;
    }


    emit test(2);
//编译过滤规则
    if (pcap_setfilter(handle, &fp) == -1) {
        qDebug() << "Couldn't install filter" << filterExp << ":" << pcap_geterr(handle);
        pcap_close(handle);
        return;
    }


   emit test(3);
//循环嗅探数据包
    while(1)
    {
        emit test(5);
        //emit emitsignal_cap(msStrPackLen,g_pPktdata);
        //qDebug()<<msStrPackLen<<endl;
        pcap_loop(handle, -1, t, (u_char *)&src_ip);
        emit test(6);

    }


//关闭过滤函数指针
    pcap_freecode(&fp);
//关闭嗅探器句柄
    pcap_close(handle);
}
