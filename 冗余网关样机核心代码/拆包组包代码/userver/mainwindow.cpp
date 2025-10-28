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

    enableSetting();

    connect(ui->pushButton_start, &QPushButton::clicked, this, &MainWindow::startListen);
    connect(ui->pushButton_stop, &QPushButton::clicked, this, &MainWindow::stopListen);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &MainWindow::clearList);
    connect(ui->pushButton_showDialog, &QPushButton::clicked, this, &MainWindow::showDialog);
    connect(ui->pushButton_updateInterface, &QPushButton::clicked, this, &MainWindow::getLocalHostIP);

    ui->progressBar_log->setMaximum(LOG_LINES_PER_FILE);
    startWorkerThread();


}

MainWindow::~MainWindow()
{
    closeWorkerThread();
    delete ui;
}

/* get all ip address */
void MainWindow::getLocalHostIP()
{
    IPlist.clear();
    ui->comboBox_server_interface->clear();

    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface interface, list) {
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        foreach (QNetworkAddressEntry entry, entryList) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QString item = QString("%1   %2").arg(interface.name()).arg(entry.ip().toString());
                ui->comboBox_server_interface->addItem(item);
                IPlist<<entry.ip();
            }
        }
    }
}

void MainWindow::startListen()
{
    int index = ui->comboBox_server_interface->currentIndex();
    if (index != -1) {
        isFirstPacket = true;

        server_ip = IPlist[index];
        server_port = ui->spinBox_port->value();
        emit signalToStartListen(server_ip, server_port);

        /* update ui */
        disableSetting();

        startTime = QDateTime::currentDateTime();
        QString str = startTime.toString("yyyy-MM-dd hh:mm:ss");
        ui->label_startTime->setText(str);

        clockTimer.setInterval(1000);
        clockTimer.setSingleShot(false);
        connect(&clockTimer, &QTimer::timeout, this, &MainWindow::clockTick);
        clockTimer.start();
    }
}
void MainWindow::stopListen()
{
    enableSetting();

    emit signalToStopListen();

    clockTimer.stop();
    disconnect(&clockTimer);
}
void MainWindow::clearList()
{
    ui->textBrowser->clear();
}
void MainWindow::messageReceived(int msg_id, long dt_us, int msg_len, QByteArray msg)
{
    long dt_ms = dt_us/1000;
    QString log = QString("%1 id=%2   len=%3   dt=%4ms =%5us").arg(timeString()).arg(msg_id).arg(msg_len).arg(dt_ms).arg(dt_us);
//    if (ui->checkBox_refresh->checkState() == Qt::CheckState::Checked) {
//        ui->textBrowser->append(log);
//    }

    currentLogLines++;
    if (currentLogLines % 100 == 0) {
        ui->progressBar_log->setValue(currentLogLines);
    }

    if (currentLogLines < LOG_LINES_PER_FILE) {
        /* write to file */
        *logOut << log << "\n";
        logOut->flush();
        /* deal with error */
        if (isFirstPacket) {
            isFirstPacket = false;
        }
        else {
            if (msg_id - last_msg_id != 1) {
                *errorOut << log << "\n";
                errorOut->flush();
                /* print log if error */
                ui->textBrowser->append(log);
            }
        }
    }
    else {
        /* save to queue */
        fullLogQueue.enqueue(log);
        /* deal with error */
        if (isFirstPacket) {
            isFirstPacket = false;
        }
        else {
            if (msg_id - last_msg_id != 1) {
                errorLogQueue.enqueue(log);
                /* print log if error */
                ui->textBrowser->append(log);
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

void MainWindow::disableSetting()
{
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);
    ui->spinBox_port->setEnabled(false);
    ui->comboBox_server_interface->setEnabled(false);
    ui->pushButton_updateInterface->setEnabled(false);
}

void MainWindow::enableSetting()
{
    ui->pushButton_stop->setEnabled(false);
    ui->pushButton_start->setEnabled(true);
    ui->spinBox_port->setEnabled(true);
    ui->comboBox_server_interface->setEnabled(true);
    ui->pushButton_updateInterface->setEnabled(true);
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
void MainWindow::startWorkerThread()
{
    /* create worker and move to worker thread */
    Worker *worker = new Worker;
    worker->moveToThread(&workerThread);

    connect(this, &MainWindow::signalToStartListen, worker, &Worker::startListen);
    connect(this, &MainWindow::signalToStopListen, worker, &Worker::stopListen);
    connect(worker, &Worker::messageReceived, this, &MainWindow::messageReceived);
    connect(worker, &Worker::udpSocketInitialized, this, &MainWindow::udpSocketInitialized);

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

}
void MainWindow::listenStopped()
{
    ui->pushButton_stop->setEnabled(false);
    ui->pushButton_start->setEnabled(true);
    QString log = QString("%1 停止监听").arg(timeString());
    ui->textBrowser->append(log);
    *logOut << log << "\n";
    logOut->flush();
    closeLogFile();
}
void MainWindow::udpSocketInitialized()
{
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);
    QString log = QString("%1 开始监听").arg(timeString());
    ui->textBrowser->append(log);

    openNewLogFile();
    *logOut << log << "\n";
    logOut->flush();
}
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
    logFile = new QFile(dir.filePath("server_full_log_%1.txt").arg(dateTimeString()));
    QFileInfo logFileInfo(*logFile);
    logPath = logFileInfo.filePath();

    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        ui->textBrowser->append("Failed to Open fullLog");
    }
    else {
        logOut = new QTextStream(logFile);
    }

    errorFile = new QFile(dir.filePath("server_error_log_%1.txt").arg(dateTimeString()));
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
void MainWindow::messageReceivedSimple(QByteArray ba, int dt_us)
{
    QString log = QString("%1   %2us   %3").arg(timeString()).arg(dt_us).arg(QString(ba));
    ui->textBrowser->append(log);
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
