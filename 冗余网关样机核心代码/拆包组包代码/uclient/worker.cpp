#include "worker.h"

Worker::Worker(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Worker::invoke);
    msg_id = 1;
}
Worker::~Worker()
{
    udpSocket->close();
    udpSocket->deleteLater();
}
void Worker::setServerInfo(QHostAddress _server_ip, int _server_port, QHostAddress _client_ip)
{
    server_ip = _server_ip;
    client_ip = _client_ip;
    server_port = _server_port;
    udpSocket->bind(client_ip);
}
void Worker::toStartTimer(int _period, int _packet_len)
{
    is_start = true;
    packet_len = _packet_len;
    send_buff.resize(packet_len); /* resize the send buffer to given packet length */
    send_buff = send_buff.fill(0xFF); /* fill the send buffer with 0xFF */
    memcpy(&send_buff.data()[0], &packet_len, sizeof(packet_len)); /* write packet length to the first 2 bytes of send buffer */
    period_ms = _period;
    timer->start(period_ms);
}
void Worker::toStopTimer()
{
    timer->stop();
    udpSocket->close();
}
void Worker::invoke()
{
    memcpy(&send_buff.data()[4], &msg_id, sizeof(msg_id));
    udpSocket->writeDatagram(send_buff, server_ip, server_port);

    /* update standard time */
    long dt_us = period_ms*1000;
    if (is_start) {
        clock_gettime(CLOCK_MONOTONIC, &lastTime);
        is_start = false;
    }
    else {
        struct timespec realTime;
        clock_gettime(CLOCK_MONOTONIC, &realTime);

        dt_us = find_dt_us(&lastTime, &realTime);
        lastTime = realTime;
    }
    emit timerInvoked(msg_id, dt_us, packet_len, send_buff);
    msg_id++;
    msg_id %= INT_MAX;
}
/* delta time between time1 and time2 in us */
long Worker::find_dt_us(struct timespec *time1, struct timespec *time2)
{
    long ds = time2->tv_sec - time1->tv_sec;
    long dns = time2->tv_nsec - time1->tv_nsec;
    long dt_us = ds * 1000000 + dns / 1000;
    return dt_us;
}
void Worker::toResetMsgId()
{
    msg_id = 1;
}
