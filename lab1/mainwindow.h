#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <ui_mainwindow.h>
#include <string>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static QString ip;       //ip地址
    static QString port;     //端口号
    static QString filetable;//文件目录
    static QString out_ip;
    static QString out_port;
    static QString out_head;
    static QString output;
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
