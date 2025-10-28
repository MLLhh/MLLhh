#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <buildudp.h>

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
    UdpdataSocket *msocket;
    QHostAddress addr;
    quint16 port;
private slots:
    void listen_start();
    void listen_over();
    void receieve_start();
    void k_test();
signals:

};
#endif // MAINWINDOW_H
