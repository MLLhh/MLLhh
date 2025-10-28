#ifndef WORKER2_H
#define WORKER2_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QTimer>
#include <QTime>
#include <time.h>
//#include <limits>

#define NSEC_PER_SEC 1000*1000*1000

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker();
private:
    QUdpSocket *udpSocket;
    QTimer *timer;
    QHostAddress server_ip;
    QHostAddress client_ip;
    int server_port;
    int period_ms;
    int packet_len; /* packet length */
    int msg_id;
    QByteArray send_buff;
    struct timespec lastTime;
    bool is_start = false;
private:
    long find_dt_us(struct timespec *time1, struct timespec *time2);
signals:
    void timerInvoked(int msg_id, long dt_us, int msg_len, QByteArray send_buff);
public slots:
    void toStartTimer(int _period, int _packet_len);
    void toStopTimer();
    void setServerInfo(QHostAddress _server_ip, int _server_port, QHostAddress _client_ip);
    void invoke();
    void toResetMsgId();
};

#endif // WORKER2_H
