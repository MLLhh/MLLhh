#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QThread>
#include "worker.h"
#include <QMetaType>
#include <time.h>
#include "dialog.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QQueue>
#include <QDateTime>

#define LOG_LINES_PER_FILE 100000

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

//    QTcpServer *tcpServer;
//    QTcpSocket *tcpSocket;

    QList<QHostAddress> IPlist; /* save all ip address */
    int last_msg_id;

    /* multithreading */
    QThread workerThread;
    QHostAddress server_ip;
    int server_port;
    QHostAddress client_ip;
    int client_port;

    QFile *logFile;
    QFile *errorFile;
    QTextStream *logOut;
    QTextStream *errorOut;
    QString logPath;
    QString errorPath;

    QQueue<QString> fullLogQueue;
    QQueue<QString> errorLogQueue;
    bool logFileIsReady;
    int currentLogLines;
    QDateTime startTime;
    QTimer clockTimer;
    bool isFirstPacket;

private:
    void getLocalHostIP(); /* get all ip address */

    void disableSetting();
    void enableSetting();
    QString timeString();
    QString dateTimeString();
    void showDialog();
    void openNewLogFile();
    void closeLogFile();
    void updateDateTime();

    /* multithreading */
    void startWorkerThread();
    void closeWorkerThread();

signals:
    void signalToStartListen(QHostAddress ip, int port);
    void signalToStopListen();

private slots:
    void startListen();
    void stopListen();
    void clearList();
    void messageReceived(int msg_id, long dt_us, int msg_len, QByteArray msg);
    void udpSocketInitialized();
    void listenStopped();
    void clockTick();
    /* test */
    void messageReceivedSimple(QByteArray ba, int dt_us);

};
#endif // MAINWINDOW_H
