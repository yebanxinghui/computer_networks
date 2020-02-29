#include "Server.h"
#include <stdlib.h>

#include <iostream>
#include <QApplication>
#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <winsock2.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
