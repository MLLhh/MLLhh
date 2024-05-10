#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "udpserver.h"
#include "SocketInclude.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    UdpServer mUdpserver;
    mUdpserver.StartUp(9000);
    //startThreads();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startThreads()
{
    //UdpServer *mUdpServer = new UdpServer();
    //mUdpServer->moveToThread(&receieveThread);
}

void MainWindow::on_pushButton_clicked()
{

}
