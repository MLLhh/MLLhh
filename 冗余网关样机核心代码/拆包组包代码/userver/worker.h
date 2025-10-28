#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QTimer>
#include <QTime>
#include <time.h>
#include <QDebug>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
private:
    QUdpSocket *udpSocket = nullptr;
    QHostAddress server_ip;
    int server_port;
    struct timespec lastTime;
private:
    long find_dt_us(struct timespec *time1, struct timespec *time2);
signals:
    void clientConnected(QHostAddress client_ip, int client_port);
    void clientDisconnected(QHostAddress client_ip, int client_port);
    void messageReceived(int msg_id, long dt_us, int msg_len, QByteArray msg);
    void udpSocketInitialized();
    void listenStopped();
    /* test */
    void messageReceivedSimple(QByteArray ba, int dt_us);
private slots:
    void onReadyRead();
public slots:
    void initUdpSocket(QHostAddress _server_ip, int _server_port);
    void stopListen();
    void startListen(QHostAddress ip, int port);
};

#endif // WORKER_H
