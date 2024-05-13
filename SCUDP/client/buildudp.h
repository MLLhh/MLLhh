#ifndef BUILDUDP_H
#define BUILDUDP_H
#include <QUdpSocket>
#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>


typedef struct _PACKAGE_HEAD
{
    char magicNumber[4];        // 操作码
    quint32 BN;                 // 数据批号
    quint32 packageHeadSize;    // 包头字节数
    quint32 packageSize;        // 包内数据字节数
    quint32 packageTotal;       // 数据包个数
    quint32 fileSize;           // 数据总字节数
    quint32 packageOffset;      // 数据包偏移量
    static quint32 serialNum;   // 计数，用于生成标识同批数据的数据批号
    _PACKAGE_HEAD() {
        magicNumber[0] = uchar(0xBB);
        magicNumber[1] = uchar(0xCC);
        magicNumber[2] = uchar(0xDD);
        magicNumber[3] = uchar(0xEE);
        BN = ++serialNum;
    }
    _PACKAGE_HEAD(int) {

    }

    // 返回下一个相邻数据报的字节偏移量
    quint32 endIndex() const {
        return packageOffset + packageSize;
    }
}PackageHead;

class UdpdataSocket : public QUdpSocket
{
public:
    explicit UdpdataSocket(QObject *parent = nullptr);
    // 重载父类的同名函数，不需分包时用法相同；需要分包时，根据包容量自动分包传输
    qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port);
private:
    quint32 m_package_capacity; // 数据包容量，不含包头的数据大小
    quint32 m_packagehead_size; // 包头字节数
    qint64 m_sleep_millisecond;  // 两相邻包间的发送间隔，减少丢包
};

#endif // BUILDUDP_H
