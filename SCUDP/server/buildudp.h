#ifndef BUILDUDP_H
#define BUILDUDP_H

#include <QUdpSocket>
#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
//#include <todetector.h>

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

typedef struct _PACKAGE_STATUS
{
    quint32 BN;             // 数据批号
    quint32 packageCount;   // 已拼接包数
    quint32 packageTotal;   // 包总数
    quint32 fileSize;       // 总字节数
    QDateTime startTime;    // 接收到首个包的时刻
    char* data;             // 数据
    _PACKAGE_STATUS(const PackageHead& packHead) {
        BN = packHead.BN;
        packageCount = 0;
        packageTotal = packHead.packageTotal;
        fileSize = packHead.fileSize;
        startTime = QDateTime::currentDateTime();
        data = (char*)malloc(fileSize * sizeof(char));
    }

    // 将数据片段装入对应位置
    qint8 insertPackage(const PackageHead& packHead, char *data) {
        if (packHead.endIndex() > fileSize) {
            return -1;
        }

        memcpy(this->data + packHead.packageOffset, data + packHead.packageHeadSize, packHead.packageSize);
        if (++packageCount == packageTotal) {   // 完成组包
            return 1;
        }
        return 0;
    }
}PackageStatus;
/*
 * TimeoutDetector 超时检测
 * 同一批组包数据等待超过一定时间则进行清除
*/

class TimeoutDetector : public QThread
{
    Q_OBJECT
public:
    explicit TimeoutDetector(QObject *parent = nullptr);
    TimeoutDetector(QList<PackageStatus> *packs, qint64 seconds, QMutex *mutex);

    void run();

private:
    QList<PackageStatus> *m_packs;  // 待组包数据
    qint64 m_second_threshold;      // 时间阈值
    QMutex *m_mutex;                // 互斥锁
};

class UdpdataSocket : public QUdpSocket
{
public:
    explicit UdpdataSocket(QObject *parent = nullptr);
    // 重载父类的同名函数，接收到非分包数据时用法相同；处理分包数据时，自动组包
    qint64 readDatagram(QByteArray& data_arr, QHostAddress *address = nullptr, quint16 *port = nullptr);

private:
    bool isFragmented(char *data, qint64 length);   // 判断是否为分包数据
    bool headParser(PackageHead& packHead, char *data, qint64 length);  // 解析包头

    quint32 m_package_capacity; // 数据包容量，不含包头的数据大小
    quint32 m_packagehead_size; // 包头字节数
    qint64 m_second_threshold;  // 数据最长保留时间
    qint64 m_sleep_millisecond;  // 两相邻包间的发送间隔，减少丢包
    QMutex *m_mutex;    // 组包数据列表的互斥锁
    TimeoutDetector *m_timeDetector;  // 超时探测器

    QList<PackageStatus> *m_pend_data;   // 等待完成组包的数据
};



#endif // BUILDUDP_H
