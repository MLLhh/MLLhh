#ifndef TRANSCEIVER_H
#define TRANSCEIVER_H

#include <QObject>
#include <QUdpSocket>

class Transceiver : public QObject
{
    Q_OBJECT
public:
    Transceiver(QObject *parent, int channel);
private:
    QUdpSocket *socket = nullptr;
    int channel;
    QHostAddress local_ip;
    int local_port;
    QHostAddress remote_ip;
    int remote_port;
private:
    void wrap(char *data, int size);
    void unwrap(QNetworkDatagram datagram);

signals:
    void sigListenStarted(int channel);
    void sigListenStopped(int channel);
    void sigRead(char *data, int size);
public slots:
    void startListen(int channel, QHostAddress local_ip, int local_port, QHostAddress remote_ip, int remote_port);
    void stopListen();
    void read();
    void destroy();
    void send(char* data, int size);
};

#endif // TRANSCEIVER_H
