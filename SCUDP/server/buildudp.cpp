#include "buildudp.h"
#include "QDebug"

quint32 PackageHead::serialNum = 0;

UdpdataSocket::UdpdataSocket(QObject *parent)
    : QUdpSocket{parent}
{

    this->m_package_capacity = 500;
    this->m_packagehead_size = sizeof(PackageHead);
    this->m_second_threshold = 3;//有点问题，单位是秒不是毫秒

    this->m_mutex = new QMutex;
    this->m_pend_data = new QList<PackageStatus>;
    this->m_timeDetector = new TimeoutDetector(m_pend_data, m_second_threshold, m_mutex);
    this->m_timeDetector->start();

}

qint64 UdpdataSocket::readDatagram(QByteArray& data_arr, QHostAddress *address, quint16 *port) {
    data_arr.clear();
    // 读取套接字中待处理的UDP数据报
    qint64 maxSize = this->pendingDatagramSize();   // 待处理数据的字节数
    char *data = (char*)malloc(maxSize * sizeof(char));   // 申请内存
    qint64 readDataSize = QUdpSocket::readDatagram(data, maxSize, address, port);
    if (readDataSize == -1) {
        free(data);
        return -1;
    }
    // 不是分包数据
    if (isFragmented(data, maxSize) == false) {
        data_arr.append(data, maxSize);
        qDebug()<<"数据未分包"<<endl;
        free(data);
        return readDataSize;
    }
    /*
     * 处理分包数据
    */

    // 解析包头
    PackageHead packHead(0);
    if (headParser(packHead, data, maxSize) == false) {
        free(data);
        return -1;
    }

    qint64 ret = 0; // 返回值
    QMutexLocker locker(m_mutex);   // 上锁
    // 搜索同批数据包
    qint64 index = -1;
    for (int i = 0; i < (*m_pend_data).size(); ++i) {
        if ((*m_pend_data)[i].BN == packHead.BN) {
            index = i;
            break;
        }
    }

    // 填充数据
    if (index == -1) {  // 该批首个数据包
        PackageStatus packStatus(packHead);
        packStatus.insertPackage(packHead, data);
        (*m_pend_data).append(packStatus);
        //emit k();
    } else {
        if ((*m_pend_data)[index].insertPackage(packHead, data) == 1) {
            // 输出完整数据，并移除该PackageStatus
            data_arr.append((*m_pend_data)[index].data);
            (*m_pend_data).removeAt(index);
            ret = packHead.fileSize;
        }
    }

    free(data);
    return ret;
}


bool UdpdataSocket::isFragmented(char *data, qint64 length) {
    if (length < 4) {   // 数据长度不足
        return false;
    }

    if (data[0] == (char)0xBB
        && data[1] == (char)0xCC
        && data[2] == (char)0xDD
        && data[3] == (char)0xEE) {
        return true;
    }
    return false;
}


bool UdpdataSocket::headParser(PackageHead& packHead, char *data, qint64 length) {
    if (length < sizeof(PackageHead)){
        return false;
    }

    memcpy(&packHead, data, sizeof(PackageHead));
    return true;
}

TimeoutDetector::TimeoutDetector(QObject *parent) : QThread(parent) {

}

TimeoutDetector::TimeoutDetector(QList<PackageStatus> *packs, qint64 seconds, QMutex *mutex) {
    m_packs = packs;
    m_second_threshold = seconds;
    m_mutex = mutex;
}


void TimeoutDetector::run() {
    while(true)
    {
        m_mutex->lock();
        QDateTime curTime = QDateTime::currentDateTime();
        qDebug() << curTime.toString();
        for (int i = 0; i < m_packs->size(); ++i) {
            if (curTime.secsTo((*m_packs)[i].startTime) > m_second_threshold) {
                delete [] (*m_packs)[i].data;
                (*m_packs).removeAt(i);
            }
        }
        m_mutex->unlock();

        sleep(m_second_threshold);//有点问题，单位是秒不是毫秒
    }
}

