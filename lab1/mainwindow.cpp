#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Server.h"
#include <QPushButton>
#include <thread>
#include <QDebug>
#include <QMessageBox>
#include <iostream>
#include <QThread>
#include <algorithm>
#include <string>
using std::string;
QString MainWindow::ip;
QString MainWindow::port;
QString MainWindow::filetable;
QString MainWindow::out_ip;
QString MainWindow::out_port;
QString MainWindow::out_head;
QString MainWindow::output;
MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //qDebug() << (int)QThread::currentThreadId();
    this->setWindowTitle("Web服务端");
    Server *srv = new Server();
    connect(ui->pushButton, &QPushButton::clicked, this, [=](){
        ip = ui->lineEdit->text();
        //IP异常检测
        string s = ip.toStdString();
        s+='.';
        int num,right;
        for(int i=0;i<4;i++)
        {
            right = s.find('.');
            if(right == -1 || s[0]=='.')
            {
                QMessageBox::warning(this,"ERROR","invalid IP address!",QMessageBox::Ok);
                return 0;
            }
            try {
                num = stoi(s.substr(0,right));
            }
            catch(std::invalid_argument& e){
                QMessageBox::warning(this,"ERROR","IP include non-number!",QMessageBox::Ok);
                return 0;
            }
            catch(std::out_of_range& e){
                QMessageBox::warning(this,"ERROR","Out of range!",QMessageBox::Ok);
                return 0;
            }
            if(num>255 || num<0)
            {
                QMessageBox::warning(this,"ERROR","Out of range!",QMessageBox::Ok);
                return 0;
            }
            s = s.substr(s.find('.')+1);
        }
        port = ui->lineEdit_2->text();
        //端口异常检测
        string s2 = port.toStdString();
        try {
            num = stoi(s2);
        }
        catch(std::invalid_argument& e){
            QMessageBox::warning(this,"ERROR","include non-number!",QMessageBox::Ok);
            return 0;
        }
        if(num >= 65535 || num < 0)
        {
            QMessageBox::warning(this,"ERROR","invalid port!",QMessageBox::Ok);
            return 0;
        }
        filetable = ui->lineEdit_3->text();
        string path = filetable.toStdString();

        //qDebug() << "!";

        if (srv->WinsockStartup() == -1) return 0;
        else{
            output = "Server Socket Create OK !\n";
            ui->textBrowser->append(output);
        }
        if (srv->ServerStartup() == -1) return 0;
        else{
            output = "Winsock startup ok! \n";
            ui->textBrowser->append(output);
            output = "Server Socket Bind OK !\n";
            ui->textBrowser->append(output);
        }
        if (srv->ListenStartup() == -1) return 0;
        else{
            output = "Server Socket Listen OK!\n";
            ui->textBrowser->append(output);
        }
        //if (srv.v Loop() == -1) return 0;
        //qDebug() << "!!";
        ui->lineEdit->setReadOnly(true);
        ui->lineEdit_2->setReadOnly(true);
        ui->lineEdit_3->setReadOnly(true);
        ui->pushButton->setEnabled(false);

        //qDebug() << "!!!";
        std::thread threadLoop(&Server::Loop,srv);
        threadLoop.detach();
        connect(srv, &Server::sig, this, [=](QString ip, QString port, QString head){
           ui->textBrowser->append(ip);
           ui->textBrowser->append(port);
           ui->textBrowser->append(head);
        });
    });
    connect(ui->pushButton_2, &QPushButton::clicked, this, [=](){
        closesocket(srv->srvSocket);
        ui->lineEdit->setReadOnly(false);
        ui->lineEdit_2->setReadOnly(false);
        ui->lineEdit_3->setReadOnly(false);
        ui->pushButton->setEnabled(true);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
