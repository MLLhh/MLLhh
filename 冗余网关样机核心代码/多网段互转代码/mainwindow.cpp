#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QNetworkInterface>
#include <transceiver.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    qRegisterMetaType<QHostAddress>("QHostAddress");

    ui->setupUi(this);

    updateInterface();

    Transceiver *tr0 = new Transceiver(nullptr, 0);
    threadCH0 = new QThread(this);
    tr0->moveToThread(threadCH0);
    threadCH0->start(QThread::Priority::TimeCriticalPriority);

//    Transceiver *tr1 = new Transceiver(nullptr, 1);
//    threadCH1 = new QThread(this);
//    tr1->moveToThread(threadCH1);
//    threadCH1->start(QThread::Priority::TimeCriticalPriority);

    connect(this, &MainWindow::sigTrSend, tr0, &Transceiver::send);
//    connect(this, &MainWindow::sigTrSend, tr1, &Transceiver::send);

    connect(tr0, &Transceiver::sigRead, this, &MainWindow::onTrRead);
//    connect(tr1, &Transceiver::sigRead, this, &MainWindow::onTrRead);

    connect(this, &MainWindow::sigStartListen, tr0, &Transceiver::startListen);
//    connect(this, &MainWindow::sigStartListen, tr1, &Transceiver::startListen);
    connect(this, &MainWindow::sigDestroyTransceiver, tr0, &Transceiver::destroy);
//    connect(this, &MainWindow::sigDestroyTransceiver, tr1, &Transceiver::destroy);

    connect(ui->pushButton_start, &QPushButton::clicked, this, &MainWindow::onButtonStart);
    connect(ui->pushButton_stop, &QPushButton::clicked, this, &MainWindow::onButtonStop);
    connect(ui->pushButton_updateInterface, &QPushButton::clicked, this, &MainWindow::updateInterface);
}
MainWindow::~MainWindow()
{
    delete ui;
    if (socket != nullptr) {
        socket->~RawSocket();
    }

    emit sigDestroyTransceiver();

    threadCH0->quit();
    threadCH1->quit();
    threadCH0->wait();
    threadCH1->wait();
}
void MainWindow::onButtonStart()
{
    std::cout << "onButtonStart" << std::endl;
    bool on = false;
    ui->pushButton_start->setEnabled(on);
    ui->pushButton_stop->setEnabled(!on);
    ui->pushButton_updateInterface->setEnabled(on);
//    ui->comboBox_in->setEnabled(on);
//    ui->comboBox_local_0->setEnabled(on);

    QString interface_in = interfaceList.at(ui->comboBox_in->currentIndex());
    socket = new RawSocket(this, interface_in.toUtf8().constData());

    connect(socket, &RawSocket::sigRead, this, &MainWindow::onRawRead);
    connect(this, &MainWindow::sigRawSend, socket, &RawSocket::send);

    QHostAddress local_ip_0 = ipList[ui->comboBox_local_ip_0->currentIndex()];
    QHostAddress local_ip_1 = ipList[ui->comboBox_local_ip_1->currentIndex()];
    int local_port_0 = ui->lineEdit_local_port_0->text().toInt();
    int local_port_1 = ui->lineEdit_local_port_1->text().toInt();
    QHostAddress remote_ip_0 = QHostAddress(ui->lineEdit_remote_ip_0->text());
    QHostAddress remote_ip_1 = QHostAddress(ui->lineEdit_remote_ip_1->text());
    int remote_port_0 = ui->lineEdit_remote_port_0->text().toInt();
    int remote_port_1 = ui->lineEdit_remote_port_1->text().toInt();

    // todo:检查IP与端口的合法性
    emit sigStartListen(0, local_ip_0, local_port_0, remote_ip_0, remote_port_0);
    emit sigStartListen(1, local_ip_1, local_port_1, remote_ip_1, remote_port_1);
}
void MainWindow::onButtonStop()
{
    socket->disconnect();
    socket->~RawSocket();
    socket = nullptr;
    bool on = true;
    ui->pushButton_start->setEnabled(on);
    ui->pushButton_stop->setEnabled(!on);
    ui->pushButton_updateInterface->setEnabled(on);
//    ui->comboBox_in->setEnabled(on);
//    ui->comboBox_out_0->setEnabled(on);
//    ui->comboBox_out_1->setEnabled(on);
}
void MainWindow::updateInterface()
{
    std::cout << "updateInterface" << std::endl;
    interfaceList.clear();
    ipList.clear();
    ui->comboBox_in->clear();
    ui->comboBox_local_ip_0->clear();
    ui->comboBox_local_ip_1->clear();

    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, list) {
        if (interface.name() == "lo") continue;
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        foreach (QNetworkAddressEntry entry, entryList) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QString interfaceName = interface.name();
                QString ipString = entry.ip().toString();
                QString item = interfaceName + "   " + ipString;
                interfaceList << interfaceName;
                ipList << entry.ip();
                ui->comboBox_in->addItem(item);
                ui->comboBox_local_ip_0->addItem(item);
                ui->comboBox_local_ip_1->addItem(item);
            }
        }
    }
}
void MainWindow::onRawRead(char *data, int size)
{
    emit sigTrSend(data, size);
}
void MainWindow::onTrRead(char *data, int size)
{
    emit sigRawSend(data, size);
}
