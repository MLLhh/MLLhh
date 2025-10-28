#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QtMath>

namespace Ui {
class Dialog;
}

class Dialog : public QWidget
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr, QString _fullLogPath = "", QString _errorLogPath = "");
    ~Dialog();

private:
    Ui::Dialog *ui;
    QString fullLogPath;
    QString errorLogPath;
    int currentPage;
    int totalPages;
    int totalLines;
    bool fileIsReady;
    QString filePath;

private:
    void loadLines(int startLine, int endLine);
    void loadPage(int page);
    void showFullLog();
    void showErrorLog();
    void openFile();
    void prevPage();
    void nextPage();
    void prevTenPage();
    void nextTenPage();
    void firstPage();
    void lastPage();
    void loadFile(QString path);
    void update();
};

#endif // DIALOG_H
