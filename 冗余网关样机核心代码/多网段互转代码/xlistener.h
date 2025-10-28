#ifndef XLISTENER_H
#define XLISTENER_H

#include <QObject>

#define MAX_BUFFER_SIZE 65535

class XListener : public QObject
{
    Q_OBJECT
public:
    XListener(QObject *parent, int sockfd);
    ~XListener();
    void loop();
    static void staticSignalHandler(int signum);
private:
    int sockfd;
    char buffer[MAX_BUFFER_SIZE];
signals:
    void sigRxData(char *data, int size);
};

#endif // XLISTENER_H
