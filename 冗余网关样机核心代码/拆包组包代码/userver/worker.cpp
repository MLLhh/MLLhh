#include "worker.h"

Worker::Worker(QObject *parent) : QObject(parent)
{

}

/* delta time between time1 and time2 in us */
long Worker::find_dt_us(struct timespec *time1, struct timespec *time2)
{
    long ds = time2->tv_sec - time1->tv_sec;
    long dns = time2->tv_nsec - time1->tv_nsec;
    long dt_us = ds * 1000000 + dns / 1000;
    return dt_us;
}
void Worker::initUdpSocket(QHostAddress _server_ip, int _server_port)
{
    server_ip = _server_ip;
    server_port = _server_port;
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(server_ip, server_port);
    connect(udpSocket, &QUdpSocket::readyRead, this, &Worker::onReadyRead);
    emit udpSocketInitialized();
}
void Worker::stopListen()
{
    udpSocket->close();
    udpSocket->deleteLater();
    emit listenStopped();
}
void Worker::onReadyRead()
{
    QByteArray cache;
    QHostAddress clientIp;
    quint16 clientPort;

    while (udpSocket->hasPendingDatagrams()) {
        qint64 size = udpSocket->pendingDatagramSize();
        cache.resize(size);
        udpSocket->readDatagram(cache.data(), size, &clientIp, &clientPort);

        /* time */
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long dt_us = find_dt_us(&lastTime, &now);
        lastTime = now;

        /* msg_id and msg_len */
        int msg_len;
        int msg_id;
        memcpy(&msg_len, &cache.data()[0], sizeof(int));
        memcpy(&msg_id, &cache.data()[4], sizeof(int));

        emit messageReceived(msg_id, dt_us, size, cache);
    }
}
void Worker::startListen(QHostAddress ip, int port)
{
    initUdpSocket(ip, port);
}
