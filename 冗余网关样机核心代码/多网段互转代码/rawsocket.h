#ifndef RAWSOCKET_H
#define RAWSOCKET_H

#include <QObject>
#include <netpacket/packet.h>

class RawSocket : public QObject
{
    Q_OBJECT
public:
    explicit RawSocket(QObject *parent, const char *interface1);
    ~RawSocket();
private:
    int sockfd1, sockfd2;
    int ifindex1, ifindex2;
    QThread *rxThread1, *txThread1;
    QThread *rxThread2, *txThread2;
private:
    void initSocket(int &sockfd1, const char* interface, int &ifindex);
    void start();
signals:
    void sigStart();
    void sigRead(char *data, int size);
    void sigSend(char *data, int size);
public:
    void analyze(char *data, int size);
    void stop();
public slots:
    void read(char *data, int size);
    void send(char *data, int size);
};

#endif // RAWSOCKET_H
