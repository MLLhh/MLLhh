#include "transceiver.h"
#include <QUdpSocket>
#include <iostream>
#include <QNetworkDatagram>
#include <utils.h>
#include <QDataStream>

// 发送与接收
Transceiver::Transceiver(QObject *parent, int channel) : QObject(parent)
{
    this->channel = channel;
    std::cout << "Transceiver " << channel << " created" << std::endl;
}
void Transceiver::startListen(int channel, QHostAddress local_ip, int local_port, QHostAddress remote_ip, int remote_port)
{
    if (channel == this->channel) {
        std::cout << "Transceiver::startListen " << channel << std::endl;
        this->local_ip = local_ip;
        this->local_port = local_port;
        this->remote_ip = remote_ip;
        this->remote_port = remote_port;
        socket = new QUdpSocket(this);
        socket->bind(local_ip, local_port);
        connect(socket, &QUdpSocket::readyRead, this, &Transceiver::read);
        emit sigListenStarted(channel);
    }
}
void Transceiver::stopListen()
{
    socket->close();
    socket->deleteLater();
    emit sigListenStopped(channel);
}
void Transceiver::read()
{
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        unwrap(datagram);
    }
//    int size = socket->pendingDatagramSize();

//    char *data = new char[size];
//    int ret = socket->readDatagram(data, size);
//    std::cout << "read " << ret << "/" << size << std::endl;
//    emit sigRead(data, size);
}
void Transceiver::destroy()
{
    if (socket != nullptr) {
        socket->close();
        socket->deleteLater();
    }
}
void Transceiver::send(char* data, int size)
{
    wrap(data, size);
//    int ret = socket->writeDatagram(data, size, remote_ip, remote_port);
//    std::cout << "send " << size << std::endl;
}
void Transceiver::wrap(char *data, int size)
{
    std::cout << "send payload:" << std::endl;
    Utils::printHex(data, size);
//    QByteArray payload(data, size);
    QByteArray payload;
    QByteArray qdata(data, size);
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream << size;
    stream << qdata;
    QNetworkDatagram datagram(payload, remote_ip, remote_port);
    std::cout << "send datagram:" << std::endl;
    Utils::printDatagram(datagram);
    socket->writeDatagram(datagram);
}
void Transceiver::unwrap(QNetworkDatagram datagram)
{
    std::cout << "recv datagram:" << std::endl;
    Utils::printDatagram(datagram);
    QByteArray payload = datagram.data();
    QDataStream stream(&payload, QIODevice::ReadOnly);
    int size;
    stream >> size;
    QByteArray qdata;
    stream >> qdata;
    std::cout << "recv payload:" << std::endl;
    Utils::printHex(qdata.data(), size);
    emit sigRead(qdata.data(), size);
}
