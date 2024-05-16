#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <buildudp.h>
#include <QTimer>
#include <QTime>
#include <time.h>
#include <vector>
#include <random>
#include <QtCore/QRandomGenerator>

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
    QList<QHostAddress> IPlist;
    QUdpSocket *msocket;
    UdpdataSocket *bsocket;
    QByteArray sendbuff;
    QTimer *timer;
    bool is_start = false;
    void getLocalHostIP();
private slots:
    void senddata();
    void ondeviceChanged(int index);
    void toStartTimer();
    void toStopTimer();
};
#endif // MAINWINDOW_H
