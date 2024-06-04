#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "packetcapture.h"
extern const u_char* g_pPktdata;
int g_nArray[9999][9999];
int g_nLenPktData[9999][9999];
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    packetCapture = new PacketCapture(this);
    getLocalHostIP();

    connect(ui->comboBox_interface, SIGNAL(currentIndexChanged(int)), this, SLOT(onNetworkInterfaceChanged(int)));
    connect(packetCapture, SIGNAL(emitsignal_cap(QString,const u_char*)), this, SLOT(printTableInfo(QString,const u_char*)));
    // 设置tableWidgetItem 点击事件
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(on_tableWidget_cellClicked(int,int)));
}

MainWindow::~MainWindow()
{
    packetCapture->quit();

    /* 阻塞等待 2000ms 检查一次进程是否已经退出 */
    if (packetCapture->wait(2000)) {
        qDebug()<<"线程已经结束！"<<endl;
    }
    delete ui;
}



/* get all ip address */
void MainWindow::getLocalHostIP()
{
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface interface, list) {
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        foreach (QNetworkAddressEntry entry, entryList) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                ui->comboBox_interface->addItem(interface.name());
                IPlist<<entry.ip();
            }
        }
    }
}
void MainWindow::onNetworkInterfaceChanged(int index)
{
    QHostAddress ip = IPlist.at(index);
    ui->label_ip->setText(ip.toString());
}


void MainWindow::on_dialog_clicked()
{

    packetCapture->setFilePath(QFileDialog::getOpenFileName(this));

}

void MainWindow::printTableInfo(QString strLeng,const u_char* pktData)
{
    // 设置滚动条始终在底端
    ui->tableWidget->scrollToBottom();

    // 定义tableWidget中行号
    int nRow  = ui->tableWidget->rowCount();
    // 根据row依此插入
    //ui->tableWidget->insertRow(nRow);
    // 定义列表中的项目
    QTableWidgetItem *item;
    item = new QTableWidgetItem(strLeng);
    ui->tableWidget->setItem(0, 0, item);

    // 将产生的数据存放在数组中，备点击时读取
    for( int i = 0; i < strLeng.toInt(); ++i)
    {
        g_nArray[0][i] = pktData[i];

    }
    // 存储本次数据报长度
    g_nLenPktData[0][0] = strLeng.toInt();

}

void MainWindow::on_runButton_clicked()
{
    //检查线程是否在运行，如果有则结束运行
    if (!packetCapture->isRunning())
        for (int i=0;i<=3;i++) {
            qDebug()<<"启动！"<<endl;
        }

    packetCapture->set_private_var(ui->comboBox_interface->currentText(), ui->label_ip->text(), ui->lineEdit_port->text());
    // 清除内容
    ui->tableWidget->clearContents();
    // 清空数据帧列表
    ui->textEdit->clear();

    packetCapture->start(); // 启动捕获数据包的线程
}

void MainWindow::on_quitButton_clicked()
{
    // 检查线程是否在运行，如果有则结束运行
    if (packetCapture->isRunning()){
        for (int i=0;i<=3;i++)  qDebug()<<"exit！"<<endl;
        packetCapture->quit(); //结束捕获数据包的线程
    }
    else{

    }
}

void MainWindow::on_tableWidget_cellClicked(int row1, int column)
{
    int i;
    int row=0;
    char ch[10];
    ui->textEdit->clear();
    // 获取捕获包的长度
    int nPackLen = g_nLenPktData[row][0];
    //  显示捕获的数据
    for(  i = 0; i < nPackLen; i++)
    {

         // 将整型数据转换成十六进制数输出(大写)
         if(i%16 == 0)
        {
             // 每行显示16个数据
             sprintf(ch,"%04X  ",i);
             ui->textEdit->insertPlainText(QString("\n").append(ch));
         }
         sprintf(ch,"%02X ",g_nArray[row][i]);
         ui->textEdit->insertPlainText(ch);
    }
    ui->textEdit->insertPlainText("\n");
    // 打印字符信息
    for(  i = 0; i < nPackLen; i++)
    {

        if(i%16 == 0)
       {
            ui->textEdit->insertPlainText(QString("\n     "));
        }
        if( (g_nArray[row][i] < 40 ) || (g_nArray[row][i] > 126 ))
        {
            ui->textEdit->insertPlainText(" .");
        }
        sprintf(ch," %c ",g_nArray[row][i]);
        ui->textEdit->insertPlainText(ch);
    }
}
