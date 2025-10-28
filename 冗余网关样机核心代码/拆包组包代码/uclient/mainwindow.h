#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QThread>
#include "worker.h"
#include <QMetaType>
#include <time.h>
#include <QFile>
#include <QTextStream>
#include "dialog.h"
#include <QDir>
#include <QQueue>

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

    /* save all available ip */
    QList<QHostAddress> IPlist;

    int period;
    int last_msg_id = 0;
    int start_msg_id;

    /* save log to file */
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

private:
    void getLocalHostIP();
    QString timeString();
    QString dateTimeString();
    void toResetMsgId();
    void enableSetting();
    void disableSetting();
    void updateDateTime();

    /* log dialog window */
    void showDialog();
    void openNewLogFile();
    void closeLogFile();

    /* multithreading */
    QThread workerThread;

    /* Test */
    QUdpSocket socket;

signals:
    void signalToStartTimer(int period, int packet_len);
    void signalToStopTimer();
    void signalToResetMsgId();
    void signalToSetServerInfo(QHostAddress server_ip, int server_port, QHostAddress client_ip);

private slots:
    void onServerInterfaceChanged(int index);

    void toClearList();

    /* multithreading */
    void startWorkerThread();
    void closeWorkerThread();
    void toStartTimer();
    void toStopTimer();
    void timerInvoked(int msg_id, long dt_us, int msg_len, QByteArray send_buff);
    void clockTick();
    /* test */
    void send();

};
#endif // MAINWINDOW_H
