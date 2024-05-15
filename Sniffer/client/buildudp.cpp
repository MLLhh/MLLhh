#include "buildudp.h"
#include "QDebug"
quint32 PackageHead::serialNum = 0;

UdpdataSocket::UdpdataSocket(QObject *parent)
    : QUdpSocket{parent}
{
    this->m_package_capacity = 900;
    this->m_packagehead_size = sizeof(PackageHead);
    this->m_sleep_millisecond = 1;
}


qint64 UdpdataSocket::writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port) {
    if (datagram.size() < m_package_capacity) {
        qDebug()<<"文件过小"<<endl;
        return QUdpSocket::writeDatagram(datagram, host, port);
    }


    /*
     * 如果数据长度超出单个数据报限制，则进行分包发送
    */

    // 计算包数量
    quint32 packageTotalNum = datagram.size() / m_package_capacity;
    packageTotalNum += datagram.size() % m_package_capacity ? 1 : 0;
    // 分包发送
    qint64 nBytes = 0; // 发送总字节数
    PackageHead packHead;
    packHead.packageHeadSize = m_packagehead_size;
    packHead.packageTotal = packageTotalNum;
    packHead.fileSize = datagram.size();

    for (quint32 iPkg = 0; iPkg < packageTotalNum; ++iPkg) {
        // 计算偏移量
        packHead.packageOffset = iPkg * m_package_capacity;

        // 计算数据包字节数
        if (iPkg != packageTotalNum - 1) {
            packHead.packageSize = m_package_capacity;
        } else {
            packHead.packageSize = datagram.size() % m_package_capacity;
        }

        // 拼接包头包体
        QByteArray packBa;
        packBa.append((char*)&packHead, sizeof(packHead));
        packBa.append(datagram.mid(packHead.packageOffset, packHead.packageSize));

        // 发送
        qint64 len = QUdpSocket::writeDatagram(packBa, host, port);
        qDebug() << "write\t" << packageTotalNum << "\t" << iPkg << "\t" << len<< "批号\t" <<packHead.BN;
        //QThread::msleep(m_sleep_millisecond);
        if (len == -1) {
            qDebug() << "UdpImgSocket::writeDatagram: error in send " + packBa.toHex();
        } else {
            nBytes += packHead.packageSize;
        }
    }
    return nBytes;
}


