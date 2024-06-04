#include<packetcapture.h>
#include<mainwindow.h>
const u_char* g_pPktdata;
QString PacketCapture::msStrPackLen = "";
void PacketCapture::set_private_var(const QString &device, const QString &IpAddress, const QString &Port) {
    dev = device;
    ipAddress = IpAddress;
    port = Port;
}
int PacketCapture::get_private_var() {
    qDebug() << "device" << dev;
    qDebug() << "IpAddress" << ipAddress;
    qDebug() << "Port" << port;
}
void PacketCapture::setFilePath(const QString &Address)
{
    filePath = Address;
}
void PacketCapture::packetcapture(QString len,const u_char* data)
{
    emit emitsignal_cap(len,data);
}

void PacketCapture::packetHandler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{

    struct sockaddr_in *src_ip = (struct sockaddr_in *)user;
    struct ethhdr *eth = (struct ethhdr *)bytes;
    struct iphdr *ip = (struct iphdr *)(bytes + sizeof(struct ethhdr));

    struct sockaddr_in dst_ip;
    dst_ip.sin_addr.s_addr = ip->daddr;


    g_pPktdata = bytes;
    msStrPackLen = QString::number(h->len);
    PacketCapture *packagecapture1 = reinterpret_cast<PacketCapture*>(user);
    packagecapture1->packetcapture(msStrPackLen,g_pPktdata);
    if (src_ip->sin_addr.s_addr == dst_ip.sin_addr.s_addr) {
        qDebug() << "Ethernet Header:";

        qDebug() << "   |-Destination Address :" << QString("%1-%2-%3-%4-%5-%6")
                    .arg(eth->h_dest[0], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[1], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[2], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[3], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[4], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[5], 2, 16, QLatin1Char('0'));
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
void PacketCapture::run(){

    char errbuf[PCAP_ERRBUF_SIZE];

    get_private_var();
    pcap_t *handle = pcap_open_live(dev.toStdString().c_str(), BUFSIZ1, 1, 1000, errbuf);
    if (handle == NULL) {
        qDebug() << "Couldn't open device" << dev << ":" << errbuf;
        return;
    }

    struct sockaddr_in src_ip;
    inet_aton(ipAddress.toStdString().c_str(), &src_ip.sin_addr);

    struct bpf_program fp;
    bpf_u_int32 maskp, netp;
    if (pcap_lookupnet(dev.toStdString().c_str(), &netp, &maskp, errbuf) == -1) {
        qDebug() << "Can't get netmask for device" << dev;
        netp = 0;
        maskp = 0;
    }

    QString filterExp = QString("ip host %1").arg(ipAddress);

    if ((pcap_compile(handle, &fp, filterExp.toStdString().c_str(), 0, netp) == -1)){

        qDebug() << "Couldn't parse filter" << filterExp << ":" << pcap_geterr(handle);
        pcap_close(handle);
        return;
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        qDebug() << "Couldn't install filter" << filterExp << ":" << pcap_geterr(handle);
        pcap_close(handle);
        return;
    }


    pcap_loop(handle, -1, packetHandler, reinterpret_cast<u_char*>(this));
    pcap_freecode(&fp);
    pcap_close(handle);
}
