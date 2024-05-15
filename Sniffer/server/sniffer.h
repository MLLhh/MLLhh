#ifndef SNIFFER_H
#define SNIFFER_H

#include <QObject>
#include <pcap.h>
#include <iostream>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <unistd.h>
//namespace std {class Sniffer;}
class Sniffer : public QObject
{
    Q_OBJECT
public:
    explicit Sniffer(QObject *parent = nullptr);
    ~Sniffer();

private: /* variables */
    void print_hex_data(const unsigned char* data, ssize_t size);


public: /* functions */

signals:

public slots:
    void startSniffer();
};

#endif // SNIFFER_H
