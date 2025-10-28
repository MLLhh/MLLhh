#ifndef XSENDER_H
#define XSENDER_H

#include <QObject>
#include <netpacket/packet.h>

class XSender : public QObject
{
    Q_OBJECT
public:
    XSender(QObject *parent, int sockfd, int ifindex);
    ~XSender();
private:
    int sockfd;
    struct sockaddr_ll sa;
public slots:
    void send(char *data, int size);
};

#endif // XSENDER_H
