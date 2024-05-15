#include "mainwindow.h"
#include "ui_mainwindow.h"

qint64 i=0;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //msocket = new QUdpSocket(this);
    msocket = new UdpdataSocket(this);
    connect(ui->listen_start,&QPushButton::clicked,this,&MainWindow::listen_start);
    connect(ui->listen_over,&QPushButton::clicked,this,&MainWindow::listen_over);
    connect(msocket,&QUdpSocket::readyRead,this,&MainWindow::receieve_start);
    connect(ui->sniffer_pushButton,&QPushButton::clicked,this,&MainWindow::snifferthread_start);
    //connect(ui->sniffer1_pushButton,&QPushButton::clicked,this,&MainWindow::snifferthread_stop);

    thread_start();
}

MainWindow::~MainWindow()
{
    delete msocket;
    delete ui;
    snifferthread_stop();
}

void MainWindow::listen_start()
{
    msocket->bind(QHostAddress("10.0.2.15"), 10000);
    ui->spinBox_port->setEnabled(false);
    ui->listen_start->setEnabled(false);
    ui->textBrowser->append("监听端口:10000;监听IP：10.0.2.15");
}
void MainWindow::listen_over()
{
    msocket->disconnectFromHost();
    ui->spinBox_port->setEnabled(true);
    ui->listen_start->setEnabled(true);

}
void MainWindow::receieve_start()
{
    //数据缓冲区
        QByteArray arr;
        //hasPendingDatagrams()检查当前是否有待处理的数据报;
//        while(msocket->hasPendingDatagrams())
//        {
            //调整缓冲区的大小和收到的数据大小一致
            arr.resize(msocket->bytesAvailable());

            //接收数据
            //qint64 len = msocket->readDatagram(arr.data(),arr.size(),&addr,&port);
            qint64 len = msocket->readDatagram(arr);
            if(len>0){
            ++i;
            QString len_str = QString::number(len);
            QString i_str = QString::number(i);
            //显示
            ui->textBrowser->insertPlainText("单包大小:"+len_str+"编号"+i_str+"\n");
            }else{

            }
            //使能发送按钮和编辑框
            ui->spinBox_port->setEnabled(false);
            ui->listen_start->setEnabled(false);
//        }

}
//多线程
void MainWindow::thread_start()
{
    Sniffer *sniffer = new Sniffer();
    sniffer->moveToThread(&sniffraw);
    connect(this, &MainWindow::signalStartSniffer, sniffer, &Sniffer::startSniffer);
    connect(&sniffraw, &QThread::started, this, &MainWindow::onSnifferThreadStarted);
    sniffraw.start();
}
void MainWindow::snifferthread_start()
{
    ui->sniffer_pushButton->setEnabled(false);
    emit signalStartSniffer();
}
void MainWindow::onSnifferThreadStarted()
{
    qDebug()<<"Sniffer线程启动"<<endl;
}
void MainWindow::snifferthread_stop()
{
    /* disconnect signals */
    sniffraw.disconnect();
    sniffraw.quit();
    sniffraw.wait();
    qDebug()<<"Sniffer线程停止"<<endl;
}
