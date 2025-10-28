#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* this is for using QHostAddress as parameter of signal */
    qRegisterMetaType<QHostAddress>("QHostAddress");

    getLocalHostIP();

    connect(ui->pushButton_start, &QPushButton::clicked, this, &MainWindow::toStartTimer);
    connect(ui->pushButton_stop, &QPushButton::clicked, this, &MainWindow::toStopTimer);

    connect(ui->pushButton_resetMsgId, &QPushButton::clicked, this, &MainWindow::toResetMsgId);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &MainWindow::toClearList);
    connect(ui->pushButton_showLog, &QPushButton::clicked, this, &MainWindow::showDialog);

    connect(ui->comboBox_server_interface, SIGNAL(currentIndexChanged(int)), this, SLOT(onServerInterfaceChanged(int)));
    connect(ui->pushButton_updateInterface, &QPushButton::clicked, this, &MainWindow::getLocalHostIP);

    /* select default network interface */
//    ui->comboBox_client_interface->setCurrentIndex(1);
//    ui->comboBox_server_interface->setCurrentIndex(0);
//    onServerInterfaceChanged(0);
//    ui->comboBox_server_interface->setCurrentText("enp0s8");
//    ui->comboBox_client_interface->setCurrentText("enp0s9");

    startWorkerThread();

    /* update ui */
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
    ui->pushButton_resetMsgId->setEnabled(true);

    enableSetting();

//    QString log;
//    log = QString("int max = %1").arg(std::numeric_limits<int>::max());
//    ui->textBrowser->append(log);
//    log = QString("int min = %1").arg(std::numeric_limits<int>::min());
//    ui->textBrowser->append(log);
//    ui->textBrowser->append(QString::number(INT_MAX));
//    ui->textBrowser->append(QString::number(INT_MIN));

    ui->progressBar_log->setMaximum(LOG_LINES_PER_FILE);
}

MainWindow::~MainWindow()
{
    /* close thread before quit */
    closeWorkerThread();
    delete ui;
}

/* get all ip address */
void MainWindow::getLocalHostIP()
{
    IPlist.clear();
    ui->comboBox_server_interface->clear();
    ui->comboBox_client_interface->clear();

    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface interface, list) {
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        foreach (QNetworkAddressEntry entry, entryList) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QString item = QString("%1   %2").arg(interface.name()).arg(entry.ip().toString());
                ui->comboBox_client_interface->addItem(item);
                ui->comboBox_server_interface->addItem(item);
                IPlist<<entry.ip();
            }
        }
    }
    /* manual input ip */
    ui->comboBox_server_interface->addItem("手动输入IP");
    /* default */
    ui->comboBox_server_interface->setCurrentIndex(0);
    onServerInterfaceChanged(0);
}
void MainWindow::onServerInterfaceChanged(int index)
{
    if (index == ui->comboBox_server_interface->count()-1) {
        ui->label_server_ip->hide();
        ui->lineEdit_server_ip->show();
        ui->label_ip_txt->show();
    }
    else {
        ui->label_server_ip->hide();
        ui->lineEdit_server_ip->hide();
        ui->label_ip_txt->hide();
    }
}
void MainWindow::toClearList()
{
    ui->textBrowser->clear();
}
QString MainWindow::timeString()
{
    QTime time = QTime::currentTime();
    QString str = time.toString("[hh:mm:ss.zzz]");
    return str;
}
QString MainWindow::dateTimeString()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString str = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    return str;
}
void MainWindow::toResetMsgId()
{
    emit signalToResetMsgId();
}
/* -------------------------------------------------------------------------------
 * multithreading
 * ------------------------------------------------------------------------------- */

void MainWindow::startWorkerThread()
{
    /* create worker and move to worker thread */
    Worker *worker = new Worker;
    worker->moveToThread(&workerThread);

    /* connect signals and sockets */
    connect(this, &MainWindow::signalToStartTimer, worker, &Worker::toStartTimer);
    connect(this, &MainWindow::signalToStopTimer, worker, &Worker::toStopTimer);
    connect(this, &MainWindow::signalToResetMsgId, worker, &Worker::toResetMsgId);
    connect(worker, &Worker::timerInvoked, this, &MainWindow::timerInvoked);
    connect(this, &MainWindow::signalToSetServerInfo, worker, &Worker::setServerInfo);

    /* start worker thread and set thread priority */
    workerThread.start();
    workerThread.setPriority(QThread::TimeCriticalPriority);

    ui->textBrowser->append(QString("%1 子线程启动").arg(timeString()));
}

void MainWindow::closeWorkerThread()
{
    /* close worker thread */
    workerThread.disconnect();
    workerThread.quit();
    workerThread.wait();

    /* update ui */
    ui->textBrowser->append("线程已关闭");
}

void MainWindow::toStartTimer()
{
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);
    ui->pushButton_resetMsgId->setEnabled(false);
    ui->pushButton_updateInterface->setEnabled(false);
    QString log = QString("%1 开始发送").arg(timeString());
    ui->textBrowser->append(log);

    disableSetting();

    openNewLogFile();
    *logOut << log << "\n";
    logOut->flush();

    start_msg_id = last_msg_id;

    QHostAddress server_ip;
    if (ui->comboBox_server_interface->currentIndex() == ui->comboBox_server_interface->count()-1)
    {
        server_ip = QHostAddress(ui->lineEdit_server_ip->text());
    }
    else {
        server_ip = IPlist.at(ui->comboBox_server_interface->currentIndex());
    }

    QHostAddress client_ip = IPlist.at(ui->comboBox_client_interface->currentIndex());
    int server_port = ui->spinBox_server_port->value();
    emit signalToSetServerInfo(server_ip, server_port, client_ip);

    period = ui->lineEdit_send_period->text().toInt();
    int packet_len = ui->lineEdit_send_length->text().toInt();
    emit signalToStartTimer(period, packet_len);

    startTime = QDateTime::currentDateTime();
    QString str = startTime.toString("yyyy-MM-dd hh:mm:ss");
    ui->label_startTime->setText(str);

    clockTimer.setInterval(1000);
    clockTimer.setSingleShot(false);
    connect(&clockTimer, &QTimer::timeout, this, &MainWindow::clockTick);
    clockTimer.start();
}

void MainWindow::toStopTimer()
{
    ui->pushButton_stop->setEnabled(false);
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_resetMsgId->setEnabled(true);
    ui->pushButton_updateInterface->setEnabled(true);
    QString log = QString("%1 停止发送").arg(timeString());
    ui->textBrowser->append(log);
    *logOut << log << "\n";
    logOut->flush();
    closeLogFile();

    enableSetting();

    emit signalToStopTimer();

    clockTimer.stop();
    disconnect(&clockTimer);
}

void MainWindow::timerInvoked(int msg_id, long dt_us, int msg_len, QByteArray msg)
{
    long dt_ms = dt_us/1000;
    long ddt_us = dt_us - period*1000;
    long ddt_ms = ddt_us/1000;
    QString log = QString("%1 id=%2   len=%3   dt=%4ms =%5us\tddt=%6ms =%7us").arg(timeString()).arg(msg_id).arg(msg_len).arg(dt_ms).arg(dt_us).arg(ddt_ms).arg(ddt_us);

//    if (ui->checkBox_refresh->checkState() == Qt::CheckState::Checked) {
//        ui->textBrowser->append(log);
//    }

//    *logOut << log << "\n";
//    logOut->flush();

    /* error when ddt is too big */
    int divider = 1;
//    if (abs(ddt_us) > period*(1000/divider)) {
//        if (msg_id != start_msg_id + 1) {
//            *errorOut << log << "\n";
//            errorOut->flush();
//        }
//    }

    currentLogLines++;
    if (currentLogLines % 100 == 0) {
        ui->progressBar_log->setValue(currentLogLines);
    }

    if (currentLogLines < LOG_LINES_PER_FILE) {
        /* write to file */
        *logOut << log << "\n";
        logOut->flush();
        if (abs(ddt_us) > period*(1000/divider)) {
            if (msg_id != start_msg_id + 1) {
                *errorOut << log << "\n";
                errorOut->flush();
            }
        }
    }
    else {
        /* save to queue */
        fullLogQueue.enqueue(log);
        if (abs(ddt_us) > period*(1000/divider)) {
            if (msg_id != start_msg_id + 1) {
                errorLogQueue.enqueue(log);
            }
        }
        /* create new log file */
        if (logFileIsReady) {
            closeLogFile();
            openNewLogFile();
        }
    }

    /* save queue to log file */
    if (logFileIsReady) {
        while (!fullLogQueue.isEmpty()) {
            *logOut << fullLogQueue.dequeue() << "\n";
        }
        logOut->flush();

        while (!errorLogQueue.isEmpty()) {
            *errorOut << errorLogQueue.dequeue() << "\n";
        }
        errorOut->flush();
    }

    last_msg_id = msg_id;
}

/* todo: move socket operations to worker thread
 * 1. create socket in main thread
 * 2. set socket parameters in main thread
 * 3. move socket from main thread to worker thread
 * 4. worker thread do socket operations
 * 5. send received data from worker thread to main thread with signal
 */

/* todo:
 * start thread as soon as program starts
 * close thread before program exits
 *
 * connect to server when user click pushbutton
 * disconnect from server when user click pushbutton
 * start timer when user click pushbutton
 * stop timer when user click pushbutton
 *
 */

void MainWindow::showDialog()
{
    Dialog *dialog = new Dialog(nullptr, logPath, errorPath);
    dialog->show();
}
void MainWindow::openNewLogFile()
{
    logFileIsReady = false;
    QDir dir(QDir::home());
    if (!dir.cd("log")) {
        dir.mkpath("log");
        dir.cd("log");
    }
    logFile = new QFile(dir.filePath("client_full_log_%1.txt").arg(dateTimeString()));
    QFileInfo logFileInfo(*logFile);
    logPath = logFileInfo.filePath();

    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        ui->textBrowser->append("Failed to Open fullLog");
    }
    else {
        logOut = new QTextStream(logFile);
    }

    errorFile = new QFile(dir.filePath("client_error_log_%1.txt").arg(dateTimeString()));
    QFileInfo errorFileInfo(*errorFile);
    errorPath = errorFileInfo.filePath();

    if (!errorFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        ui->textBrowser->append("Failed to Open errorLog");
    }
    else {
        errorOut = new QTextStream(errorFile);
    }
    currentLogLines = 0;
    logFileIsReady = true;
}
void MainWindow::closeLogFile()
{
    logFileIsReady = false;
    logOut->flush();
    errorOut->flush();
    logFile->close();
    errorFile->close();
}
void MainWindow::send()
{
    QHostAddress server_ip = IPlist.at(ui->comboBox_server_interface->currentIndex());
    QHostAddress client_ip = IPlist.at(ui->comboBox_client_interface->currentIndex());
    int server_port = ui->spinBox_server_port->value();
    socket.bind(client_ip);
    socket.writeDatagram("hello", server_ip, server_port);
    ui->textBrowser->append("hello");
}
void MainWindow::enableSetting()
{
    ui->comboBox_client_interface->setEnabled(true);
    ui->comboBox_server_interface->setEnabled(true);
    ui->lineEdit_server_ip->setEnabled(true);
    ui->spinBox_server_port->setEnabled(true);
    ui->lineEdit_send_period->setEnabled(true);
    ui->lineEdit_send_length->setEnabled(true);
}
void MainWindow::disableSetting()
{
    ui->comboBox_client_interface->setEnabled(false);
    ui->comboBox_server_interface->setEnabled(false);
    ui->lineEdit_server_ip->setEnabled(false);
    ui->spinBox_server_port->setEnabled(false);
    ui->lineEdit_send_period->setEnabled(false);
    ui->lineEdit_send_length->setEnabled(false);
}
void MainWindow::updateDateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = startTime.secsTo(now);
    QTime time(0,0,0);
    time = time.addSecs(secs);
    QString str = time.toString("hh:mm:ss");
    ui->label_runtime->setText(str);
}
void MainWindow::clockTick()
{
    updateDateTime();
}
