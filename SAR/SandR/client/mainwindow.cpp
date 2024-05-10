#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SocketInclude.h"
#include "UdpClient.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    UdpClient mUdpClient;
    uint32_t serverIp = inet_addr("10.0.2.15");
    mUdpClient.CreateUdpCli(serverIp,9000);
    //for(int i=0;i<1;i++)
    while(1)
    {
            mUdpClient.dealUdpSendData();
            usleep(1000 * 10);
    }

    //startThreads();
}

MainWindow::~MainWindow()
{
    closeThreads();
    delete ui;
}

void MainWindow::startThreads()
{

}
void MainWindow::closeThreads()
{
    /* disconnect signals */
    senderThread.disconnect();
    senderThread.quit();
    senderThread.wait();

}
