#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "packetcapture.h"
//extern const u_char* g_pPktdata;
const u_char* g_pPktdata;
QString number;
QString msStrPackLen= "";
MainWindow *w1=nullptr;

int g_nArray[9999][9999];//用于接收数据包的全局变量
int g_nLenPktData[9999][9999];//存储数据包相关信息的全局变量
extern pcap_t *handle;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    packetCapture = new PacketCapture(this);
    getLocalHostIP();

    connect(packetCapture, SIGNAL(test(int)), this, SLOT(test1(int)));//测试run函数内部信号与主窗口连接
    connect(packetCapture, SIGNAL(emitsignal_cap(QString,u_char*)), this, SLOT(printTableInfo(QString,const u_char*)));//子线程回调函数内部信号与主线程接收数据包函数的关联函数
    connect(ui->comboBox_interface, SIGNAL(currentIndexChanged(int)), this, SLOT(onNetworkInterfaceChanged(int)));//网卡Box和IP：label的关联函数,点击网卡输出相关IP
    //测试全局函数中获取到对象实例的指针，并使用该指针发射信号
    //connect(w1,SIGNAL(k),this,SLOT(on_pushButton_clicked));
    // 设置tableWidgetItem 点击事件


    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(on_tableWidget_cellClicked(int,int)));//主窗口tableWidget单击即可显示接收到的数据包信息

    //测试将信号发送给内部槽函数，再通过内部槽函数进行转发
    //connect(packetCapture, SIGNAL(emitsignal_cap(QString,const u_char*)), packetCapture, SLOT(slot_emitsignal_cap(QString,const u_char*)));测试回调函数内部发出信号并通过类内槽函数进行接收
    //connect(packetCapture, SIGNAL(extern_emitsignal_cap(QString,const u_char*)), this, SLOT(printTableInfo(QString,const u_char*)));测试packetCapture类内槽函数发出信号并被MainWindow槽函数接收
}
//测试run函数内部信号与主窗口连接,显示可以发出信号的位置
void MainWindow::test1(int k)
{
    qDebug()<<k<<endl;
    qDebug()<<"位置"<<k<<endl;
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
//测试回调函数写成全局函数
void t(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    //测试全局函数中获取到对象实例的指针，并使用该指针发射信号
//    MainWindow *myMainWindow = w1;
    struct sockaddr_in *src_ip = (struct sockaddr_in *)user;
    struct ethhdr *eth = (struct ethhdr *)bytes;
    struct iphdr *ip = (struct iphdr *)(bytes + sizeof(struct ethhdr));

    struct sockaddr_in dst_ip;
    dst_ip.sin_addr.s_addr = ip->daddr;


      g_pPktdata = bytes;
      msStrPackLen = QString::number(h->len);
      //测试全局函数中获取到对象实例的指针，并使用该指针发射信号
      //emit myMainWindow->k();

/*    if (src_ip->sin_addr.s_addr == dst_ip.sin_addr.s_addr) {
        qDebug() << "Ethernet Header:";
//输出包头部的目标IP地址
        qDebug() << "   |-Destination Address :" << QString("%1-%2-%3-%4-%5-%6")
                    .arg(eth->h_dest[0], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[1], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[2], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[3], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[4], 2, 16, QLatin1Char('0'))
                .arg(eth->h_dest[5], 2, 16, QLatin1Char('0'));
//输出包头部的源IP地址
        qDebug() << "   |-Source Address      :" << QString("%1-%2-%3-%4-%5-%6")
                    .arg(eth->h_source[0], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[1], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[2], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[3], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[4], 2, 16, QLatin1Char('0'))
                .arg(eth->h_source[5], 2, 16, QLatin1Char('0'));
        qDebug() << "   |-Protocol            :" << ntohs(eth->h_proto);

    }

    // 打印整个数据包内容
    qDebug() << "Packet Dump:";
    for (int i = 0; i < h->caplen; i++) {
        if (i % 16 == 0) { // 每16个字节换行
            qDebug().noquote() << endl << QString("%1: ").arg(i, 4, 16, QChar('0')); // 打印当前偏移
        }
        qDebug().noquote() << QString("%1 ").arg(bytes[i], 2, 16, QChar('0')); // 打印每个字节的十六进制值

    }
    qDebug().noquote() << endl; // 确保在数据包末尾有一个换行
*/
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
//用label显示ip信息
void MainWindow::onNetworkInterfaceChanged(int index)
{
    QHostAddress ip = IPlist.at(index);
    ui->label_ip->setText(ip.toString());
}

//打开文件并将路径保存下来
void MainWindow::on_dialog_clicked()
{

    packetCapture->setFilePath(QFileDialog::getOpenFileName(this));

}
//显示数据包长度信息并将数据包保存至全局变量
void MainWindow::printTableInfo(QString strLeng,const u_char* pktData)
{


    label6 = new QLabel(this);
    label6->setGeometry(280, 250, 400, 20);
    label6->setText("123");

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
//ui下设置的tableWidget槽函数,点击tableWidget相关行和列就会在表格和数据帧LineEdit中输出数据包和相关信息的内容
void MainWindow::on_tableWidget_cellClicked(int row, int column)
{
    int i;
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
//点击开始，运行线程
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
//点击关闭，结束线程
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

//测试显示出接收到的全局变量
void MainWindow::on_pushButton_clicked()
{
    for(int i=0;i<6;i++){
    qDebug().noquote() << QString("%1 ").arg(g_pPktdata[0], 2, 16, QChar('0')); // 打印每个字节的十六进制值
    }
    qDebug()<<"hhh"<<endl;

    //qDebug().noquote() << endl; // 确保在数据包末尾有一个换行
}
