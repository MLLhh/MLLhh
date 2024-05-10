#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SocketInclude.h"
#include "UdpClient.h"
#include "UdpServer.h"
#include "QDebug"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    UdpServer mUdpserver;
    mUdpserver.StartUp(9000);//服务端启动函数

    UdpClient mUdpClient;
    uint32_t serverIp = inet_addr("10.0.2.15");
    mUdpClient.CreateUdpCli(serverIp,9000);
    for(int i=0;i<3;i++)
    {
            mUdpClient.dealUdpSendData();//客户端启动函数
            usleep(1000 * 10);
    }



}

MainWindow::~MainWindow()
{
    delete ui;
}

