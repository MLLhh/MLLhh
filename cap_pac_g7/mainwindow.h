#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QThread>
#include <QDebug>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <pcap.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <fstream>
#include <netinet/tcp.h>
#include<QUdpSocket>
#include<QNetworkInterface>
#include<QFile>
#include <QFileDialog>
#include <QTextStream>
#include<QMutex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class PacketCapture;
void t(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes);
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void k();
private:
    Ui::MainWindow *ui;


    QList<QHostAddress> IPlist; /* save all ip address */

    PacketCapture *packetCapture;
    void getLocalHostIP();
    QLabel *label6;
private slots:
    void onNetworkInterfaceChanged(int index);
    void on_dialog_clicked();
    void printTableInfo(QString,const u_char*);
    void on_runButton_clicked();
    void on_quitButton_clicked();
    void on_tableWidget_cellClicked(int row, int column);
    void test1(int);
    void on_pushButton_clicked();
};


#endif // MAINWINDOW_H
