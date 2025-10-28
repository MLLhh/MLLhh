#include "dialog.h"
#include "ui_dialog.h"

#define LINES_PER_PAGE 1000

Dialog::Dialog(QWidget *parent, QString _fullLogPath, QString _errorLogPath) :
    QWidget(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    fullLogPath = _fullLogPath;
    errorLogPath = _errorLogPath;

    /* file control */
    connect(ui->pushButton_openFile, &QPushButton::clicked, this, &Dialog::openFile);
    connect(ui->pushButton_full_log, &QPushButton::clicked, this, &Dialog::showFullLog);
    connect(ui->pushButton_error_log, &QPushButton::clicked, this, &Dialog::showErrorLog);
    connect(ui->pushButton_update, &QPushButton::clicked, this, &Dialog::update);
    /* page control */
    connect(ui->pushButton_prev_page, &QPushButton::clicked, this, &Dialog::prevPage);
    connect(ui->pushButton_next_page, &QPushButton::clicked, this, &Dialog::nextPage);
    connect(ui->pushButton_first_page, &QPushButton::clicked, this, &Dialog::firstPage);
    connect(ui->pushButton_last_page, &QPushButton::clicked, this, &Dialog::lastPage);
    connect(ui->pushButton_prev_ten_page, &QPushButton::clicked, this, &Dialog::prevTenPage);
    connect(ui->pushButton_next_ten_page, &QPushButton::clicked, this, &Dialog::nextTenPage);

    showFullLog();
}

Dialog::~Dialog()
{
    delete ui;
}
/* load from line <startLine> to <endLine> */
void Dialog::loadLines(int startLine, int endLine)
{
    ui->textBrowser->clear();
    if (fileIsReady) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->textBrowser->append("文件打开失败");
            file.close();
        }
        else {
            QTextStream in(&file);
            int currentLine = 0;
            while (!in.atEnd()) {
                QString line = in.readLine();
                currentLine++;
                if (currentLine >= startLine && currentLine <= endLine) {
                    ui->textBrowser->append(line);
                }
                else if (currentLine > endLine) {
                    break;
                }
            }
            file.close();
        }
    }
}
/* load page <page> */
void Dialog::loadPage(int page)
{
    if (fileIsReady) {
        int startLine = LINES_PER_PAGE * page;
        int endLine = LINES_PER_PAGE * (page+1);
        loadLines(startLine, endLine);
        if (totalPages == 0) {
            ui->label_page->setText("0/0");
        }
        else {
            ui->label_page->setText(QString("%1/%2").arg(currentPage+1).arg(totalPages));
        }
    }
}
void Dialog::showFullLog()
{
    loadFile(fullLogPath);
}
void Dialog::showErrorLog()
{
    loadFile(errorLogPath);
}
void Dialog::openFile()
{
    QDir dir(QDir::home());
    dir.cd("log");
    QString path = QFileDialog::getOpenFileName(this, tr("打开文件"), dir.path(), tr("文件 (*.txt)"));
    loadFile(path);
}
void Dialog::prevPage()
{
    if (currentPage > 0) {
        currentPage--;
        loadPage(currentPage);
    }
}
void Dialog::nextPage()
{
    if (currentPage < totalPages-1) {
        currentPage++;
        loadPage(currentPage);
    }
}
void Dialog::loadFile(QString path)
{
    if (!path.isEmpty()) {
        fileIsReady = false;
        ui->textBrowser->clear();
        ui->lineEdit_filePath->setText(path);

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->textBrowser->append("文件打开失败");
            file.close();
        }
        else {
            filePath = path;
            totalPages = 0;
            totalLines = 0;
            currentPage = 0;
            QTextStream in(&file);
            while (!in.atEnd()) {
                in.readLine();
                totalLines++;
            }
            file.close();
            totalPages = qCeil(static_cast<double>(totalLines) / LINES_PER_PAGE);
            fileIsReady = true;
            currentPage = 0;
            loadPage(currentPage);
        }
    }
}
void Dialog::firstPage()
{
    currentPage = 0;
    loadPage(currentPage);
}
void Dialog::lastPage()
{
    currentPage = totalPages-1;
    loadPage(currentPage);
}
void Dialog::update()
{
    loadFile(filePath);
}
void Dialog::prevTenPage()
{
    currentPage-=10;
    if (currentPage < 0) {
        currentPage = 0;
    }
    loadPage(currentPage);
}
void Dialog::nextTenPage()
{
    currentPage+=10;
    if (currentPage >= totalPages) {
        currentPage = totalPages-1;
    }
    loadPage(currentPage);
}
