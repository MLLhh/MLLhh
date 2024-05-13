#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "buildudp.h"
int i=0;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    getLocalHostIP();
    //msocket = new QUdpSocket(this);
    bsocket = new UdpdataSocket(this);
    connect(ui->send_button,&QPushButton::clicked,this,&MainWindow::toStartTimer);
    connect(ui->stop_button,&QPushButton::clicked,this,&MainWindow::toStopTimer);
    connect(timer,&QTimer::timeout,this,&MainWindow::senddata);
    connect(ui->cb_device,SIGNAL(currentIndexChanged(int)),this,SLOT(ondeviceChanged(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::senddata()
{
    qint64 len;
    int length = 1500;
    sendbuff.resize(length);
    QByteArray arr = ui->lineEdit_send->text().toUtf8();
    //msocket->writeDatagram(arr,QHostAddress::Broadcast,ui->spinBox_port->value());
    len = bsocket->writeDatagram(sendbuff, QHostAddress("10.0.2.15"), 10000);
    QString len_str = QString::number(len);
    ++i;
    QString i_str = QString::number(i);
    ui->textBrowser->append("单包大小:"+len_str+"number"+i_str);
    //ui->textBrowser->insertPlainText("send:"+QString(arr)+"\n");

    ui->lineEdit_send->clear();
}
void MainWindow::getLocalHostIP()
{
     QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
     ui->cb_device->addItem("输入IP");
     foreach (QNetworkInterface interface, list) {
         QList<QNetworkAddressEntry> entryList = interface.addressEntries();
         foreach (QNetworkAddressEntry entry, entryList) {
             if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                 ui->cb_device->addItem(interface.name());
                 IPlist<<entry.ip();
             }
         }
     }
}
void MainWindow::ondeviceChanged(int index)
{
    if (index == 0) {
        ui->label_ip->hide();
        ui->lineEdit_ip->show();
    }
    else {
        ui->label_ip->show();
        ui->lineEdit_ip->hide();
        QHostAddress ip = IPlist.at(index-1);
        ui->label_ip->setText(ip.toString());
    }
}
void MainWindow::toStartTimer()
{
    is_start = true;
    timer->start(1*1000);
}
void MainWindow::toStopTimer()
{
    timer->stop();
}
