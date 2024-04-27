#ifndef PACKETCAPTURE_H
#define PACKETCAPTURE_H
#include <QThread>
#include <string>
#include <QDebug>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <fstream>
#include <netinet/tcp.h>
#include<QUdpSocket>
#include<QNetworkInterface>
#include<QFile>
#include <QFileDialog>
#include <QTextStream>
#include<QMutex>
#include <QMainWindow>
#include <pcap.h>


#define BUFSIZ1 65536
void packetHandler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes);
// 自定义的PacketCapture类，用于在单独的线程中捕获数据包
class PacketCapture : public QThread {
    Q_OBJECT

public:
    void setFilePath(const QString &Address);
    // 公有成员函数，用于设置私有成员变量的值
    void set_private_var(const QString &device, const QString &IpAddress, const QString &Port);
    // 公有成员函数，用于获取私有成员变量的值
    int get_private_var();
    PacketCapture(QWidget *parent = nullptr){Q_UNUSED(parent);}
    // 输出数据到终端的回调函数
    static void packetHandler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes);

    void run();
    QString dev;
    QString ipAddress;
    QString port;

    QString filePath;
    QFile file;
    QMutex fileMutex;
    static QString msStrPackLen;

signals:
    void emitsignal_cap(QString,const u_char*);
};

#endif // PACKETCAPTURE_H
