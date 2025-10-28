#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <rawsocket.h>
#include <QThread>
#include <QNetworkInterface>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    RawSocket *socket = nullptr;
    QList<QString> interfaceList;
    QList<QHostAddress> ipList;
    QThread *threadCH0, *threadCH1;

private:
    void print(QString str);
signals:
    void sigStartListen(int channel, QHostAddress local_ip, int local_port, QHostAddress remote_ip, int remote_port);
    void sigDestroyTransceiver();
    void sigTrSend(char *data, int size);
    void sigRawSend(char *data, int size);
private slots:
    void updateInterface();
    void onButtonStart();
    void onButtonStop();
    void onRawRead(char *data, int size);
    void onTrRead(char *data, int size);
};
#endif // MAINWINDOW_H
