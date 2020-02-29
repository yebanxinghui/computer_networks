#pragma once
#include <iostream>
#include <string>
#include <mainwindow.h>
#include "ui_mainwindow.h"
#include "Server.h"
#include "WinsockEnv.h"
#include "Config.h"
#include <winsock2.h>
#include <algorithm>
#include <thread>
#include <ctime>
#include <QThread>
#include <QDebug>
#include <sys/stat.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996)

Server::Server(QObject *parent) :
    QObject(parent)
{
    //this->recvBuf = new char[Config::BUFFERLENGTH]; //初始化接受缓冲区
    //memset(this->recvBuf, '\0', Config::BUFFERLENGTH);
    //this->rcvedMessages = new list<string>();
    this->sessions = new list<SOCKET>();
    this->closedSessions = new list<SOCKET>();
    this->clientAddrMaps = new map<SOCKET, string>();
    this->socketLock = new map<SOCKET, int>();
}
Server::~Server(void)
{
    /*
    //释放接受缓冲区
    if (this->recvBuf != NULL) {
        delete this->recvBuf;
        this->recvBuf = NULL;
    }
    */

    //关闭server socket
    if (this->srvSocket != NULL) {
        closesocket(this->srvSocket);
        this->srvSocket = NULL;
    }

    //关闭所有会话socket并释放会话队列
    if (this->sessions != NULL) {
        for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++)
            closesocket(*itor); //关闭会话
        delete this->sessions;  //释放队列
        this->sessions = NULL;
    }
    //释放失效会话队列
    if (this->closedSessions != NULL) {
        for (list<SOCKET>::iterator itor = this->closedSessions->begin(); itor != this->closedSessions->end(); itor++)
            closesocket(*itor); //关闭会话
        delete this->closedSessions;//释放队列
        this->closedSessions = NULL;
    }
    /*
    //释放接受消息队列
    if (this->rcvedMessages != NULL) {
        this->rcvedMessages->clear(); //清除消息队列中的消息
        delete this->rcvedMessages;	// 释放消息队列
        this->rcvedMessages = NULL;
    }
    */

    //释放客户端地址列表
    if (this->clientAddrMaps != NULL) {
        this->clientAddrMaps->clear();
        delete this->clientAddrMaps;
        this->clientAddrMaps = NULL;
    }
    //释放socket锁列表
    if (this->socketLock != NULL) {
        this->socketLock->clear();
        delete this->socketLock;
        this->socketLock = NULL;
    }

    WSACleanup(); //清理winsock 运行环境
}
//初始化Winsock
int Server::WinsockStartup() {
    if (WinsockEnv::Startup() == -1) return -1;	//初始化Winsock
    return 0;
}

/*
初始化Server，创建TCP socket，设置服务器地址，设置服务器端口号,
设置文件目录，绑定socket到IP地址和端口号。
*/
int Server::ServerStartup() {
    //创建 TCP socket
    this->srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->srvSocket == INVALID_SOCKET){
        cout << "Server socket create error !\n";
        WSACleanup();
        return -1;
    }
    cout << "Server socket create ok!\n";

    //输入服务器的IP，端口，文件目录
    cout << "set IP address:\n";
    //cin >> (this->ServerIP);
    cout << "set port:\n";
    //cin >> (this->port);
    cout << "set filetable:\n";
    //cin >> (this->Filetable);

    //设置服务器IP地址和端口号
    this->srvAddr.sin_family = AF_INET;
    this->srvAddr.sin_port = htons(MainWindow::port.toInt());
    this->Filetable = (MainWindow::filetable).toStdString();
    //this->srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//会自动找到服务器合适的IP地址
    this->srvAddr.sin_addr.S_un.S_addr = inet_addr(MainWindow::ip.toStdString().c_str()); //这是另外一种设置IP地址的方法

    //绑定 socket to Server's IP and port
    //LPSOCKADDR相当于SOCKADDR * ,前面加个LP代表结构指针
    int rtn = ::bind(this->srvSocket, (LPSOCKADDR)&(this->srvAddr), sizeof(this->srvAddr));
    if (rtn == SOCKET_ERROR) {
        cout << "Server socket bind error!\n";
        closesocket(this->srvSocket);
        WSACleanup();
        return -1;
    }

    cout << "Server socket bind ok!\n";
    return 0;
}

//开始监听,等待客户的连接请求
int Server::ListenStartup() {
    int rtn = listen(this->srvSocket, Config::MAXCONNECTION);
    if (rtn == SOCKET_ERROR) {
        cout << "Server socket listen error!\n";
        closesocket(this->srvSocket);
        WSACleanup();
        return -1;
    }

    cout << "Server socket listen ok!\n";
    return 0;
}

/*
//将收到的客户端消息保存到消息队列
void Server::AddRecvMessage(string str) {
    if (!str.empty())
        this->rcvedMessages->insert(this->rcvedMessages->end(), str);

}
*/

//将新的会话SOCKET加入队列
void Server::AddSession(SOCKET session) {
    if (session != INVALID_SOCKET) {
        this->sessions->insert(this->sessions->end(), session);
    }
}

//将失效的会话SOCKET加入队列
void Server::AddClosedSession(SOCKET session) {
    if (session != INVALID_SOCKET) {
        this->closedSessions->insert(this->closedSessions->end(), session);
    }
}

//将失效的SOCKET从会话SOCKET队列删除
void Server::RemoveClosedSession(SOCKET closedSession) {
    if (closedSession != INVALID_SOCKET) {
        list<SOCKET>::iterator itor = find(this->sessions->begin(), this->sessions->end(), closedSession);
        if (itor != this->sessions->end())
            this->sessions->erase(itor);
    }
}

//将失效的SOCKET从会话SOCKET队列删除
void Server::RemoveClosedSession() {
    for (list<SOCKET>::iterator itor = this->closedSessions->begin(); itor != this->closedSessions->end(); itor++) {
        this->RemoveClosedSession(*itor);
    }
}

//从SOCKET s接受消息
void Server::recvMessage(SOCKET socket) {
    map<SOCKET, int>::iterator it = (this->socketLock)->find(socket);
    if (it->second == 1)
    {
        cout << "this socket is being handled by other threads!" << endl;
        return;
    }
    //现在用该线程正在处理这个socket,上锁
    it->second = 1;
    //新建该进程的接收缓冲区
    char *recvBuf = new char[Config::BUFFERLENGTH];
    int receivedBytes = recv(socket, recvBuf, Config::BUFFERLENGTH, 0);
    if (receivedBytes == SOCKET_ERROR || receivedBytes == 0) {//接受数据错误，把产生错误的会话socekt加入sessionsClosed队列
        this->AddClosedSession(socket);
        //string s("来自" + this->GetClientAddress(this->clientAddrMaps, socket) + "的游客离开了聊天室,我们深深地凝望着他(她)的背影...\n");
        string s("receive message error,maybe some sockets closed!\n");
        //this->AddRecvMessage(s);
        cout << s;
    }
    else {
        recvBuf[receivedBytes] = '\0';
        //string s("来自" + this->GetClientAddress(this->clientAddrMaps, socket) + "的游客说:" + recvBuf + "\n");
        //this->AddRecvMessage(s); //将收到的消息加入到消息队列
        //cout << s;
        this->print(socket, recvBuf);	//输出接收到的请求报文的IP地址、端口号、GET请求。
        this->fileProcess(socket, recvBuf); //用于处理请求报文
    }
    memset(recvBuf, '\0', Config::BUFFERLENGTH);//清除接受缓冲区
    delete[] recvBuf;
    //开锁，该进程进程已经处理完该会话
    it->second = 0;
    string s("the thread from " + this->GetClientAddress(this->clientAddrMaps, socket) + " request ends, now close!\n");
    cout << s << endl;
}
/*
//向SOCKET s发送消息
void Server::sendMessage(SOCKET socket, string msg) {
    int rtn = send(socket, msg.c_str(), msg.length(), 0);
    if (rtn == SOCKET_ERROR) {//发送数据错误，把产生错误的会话socekt加入sessionsClosed队列
//		cout << "Send to client failed!" << endl;
//		cout << "A client leaves..." << endl;
        string s("来自" + this->GetClientAddress(this->clientAddrMaps, socket) + "的游客离开了聊天室,我们深深地凝望着他(她)的背影...\n");
        this->AddRecvMessage(s);
        this->AddClosedSession(socket);
        cout << s;
    }
}

//向其他客户转发信息
void Server::ForwardMessage() {
    if (this->numOfSocketSignaled > 0) {
        if (!this->rcvedMessages->empty()) {//如果消息队列不为空
            for (list<string>::iterator msgItor = this->rcvedMessages->begin(); msgItor != this->rcvedMessages->end(); msgItor++) {//对消息队列中的每一条消息
                for (list<SOCKET>::iterator sockItor = this->sessions->begin(); sockItor != this->sessions->end(); sockItor++) {//对会话socket队列中的每个socket
                    if (FD_ISSET(*sockItor, &this->wfds)) {
                        this->sendMessage(*sockItor, *msgItor);
                    }
                }
            }
        }
        this->rcvedMessages->clear(); //向其他客户转发消息后，清除消息队列
    }
}
*/

int Server::AcceptRequestionFromClient() {
    sockaddr_in clientAddr;		//客户端IP地址
    int nAddrLen = sizeof(clientAddr);
    u_long blockMode = Config::BLOCKMODE;//将session socket设为非阻塞模式以监听客户连接请求

    //检查srvSocket是否收到用户连接请求
    if (this->numOfSocketSignaled > 0) {
        if (FD_ISSET(this->srvSocket, &rfds)) {  //有客户连接请求到来
            this->numOfSocketSignaled--;

            //产生会话socket
            SOCKET newSession = accept(this->srvSocket, (LPSOCKADDR)&(clientAddr), &nAddrLen);
            if (newSession == INVALID_SOCKET) {
                cout << "Server accept connection request error!\n";
                return -1;
            }

            cout << "New client connection request arrived and new session created\n";

            //将新的会话socket设为非阻塞模式，
            if (ioctlsocket(newSession, FIONBIO, &blockMode) == SOCKET_ERROR) {
                cout << "ioctlsocket() for new session failed with error!\n";
                return -1;
            }

            //将新的session加入会话队列
            this->AddSession(newSession);
            this->clientAddrMaps->insert(map<SOCKET, string>::value_type(newSession, this->GetClientAddress(newSession)));//保存地址
            map<SOCKET, int>::iterator it = (this->socketLock)->find(newSession);
            if (it == (this->socketLock)->end())//如果在锁的队列中没有找到这个套接字对应的锁则创建
                this->socketLock->insert(map<SOCKET, int>::value_type(newSession, 0));//创建套接字锁
            //string s("来自" + this->GetClientAddress(this->clientAddrMaps, newSession) + "的游客进入到聊天室\n");
            //this->AddRecvMessage(s);
            //cout << s;
        }
    }
    return 0;
}

void Server::ReceieveMessageFromClients() {
    if (this->numOfSocketSignaled > 0) {
        //遍历会话列表中的所有socket，检查是否有数据到来
        for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++) {
            if (FD_ISSET(*itor, &rfds)) {  //某会话socket有数据到来
                //接受数据
                std::thread sock_thread(Server::recvMessage,this,*itor);
                sock_thread.detach();
                string s("the thread from " + this->GetClientAddress(this->clientAddrMaps, *itor) + " has closed\n");
                cout << s << endl;
            }
        }//end for
    }
}

//得到客户端IP地址
string  Server::GetClientAddress(SOCKET s) {
    string clientAddress; //clientAddress是个空字符串， clientAddress.empty() is true
    sockaddr_in clientAddr;
    int nameLen = sizeof(clientAddr);
    int rtn = getsockname(s, (LPSOCKADDR)&clientAddr, &nameLen);
    if (rtn != SOCKET_ERROR) {
        clientAddress += inet_ntoa(clientAddr.sin_addr);
    }

    return clientAddress;
}

//得到客户端IP地址
string  Server::GetClientAddress(map<SOCKET, string> *maps, SOCKET s) {
    map<SOCKET, string>::iterator itor = maps->find(s);
    if (itor != maps->end())
        return (*itor).second;
    else {
        return string("");
    }

}

//接受客户端发来的请求和数据并转发
int Server::Loop() {

    //qDebug() << (int)QThread::currentThreadId();
    u_long blockMode = Config::BLOCKMODE;//将srvSock设为非阻塞模式以监听客户连接请求
    int rtn;

    if ((rtn = ioctlsocket(this->srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
        cout << "ioctlsocket() failed with error!\n";
        return -1;
    }
    cout << "ioctlsocket() for server socket ok!Waiting for client connection and data\n";

    //time(&(this->time_begin));
    while (true) { //等待客户的连接请求
        //QThread::msleep(1000);
        //初始化任务完成了，现在开始执行循环
        //cout << "loop begin!" << endl;
        //首先从会话SOCKET队列中删除掉已经关闭的会话socket
        this->RemoveClosedSession();

        //Prepare the read and write socket sets for network I/O notification.
        FD_ZERO(&this->rfds);
        FD_ZERO(&this->wfds);

        //把srvSocket加入到rfds，等待用户连接请求
        FD_SET(this->srvSocket, &this->rfds);

        //把当前的会话socket加入到rfds,等待用户数据的到来;加到wfds，等待socket可发送数据
        for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++) {
            FD_SET(*itor, &rfds);
            FD_SET(*itor, &wfds);
        }
        //tv.tv_sec=10 秒    tv.tv_usec = 0 微秒
        //struct timeval tv = { 10, 0 };
        //等待用户连接请求或用户数据到来或会话socket可发送数据
        //select函数返回值：负值：select错误,正值：某些文件可读写或出错,0：等待超时，没有可读写或错误的文件
        this->numOfSocketSignaled = select(0, &this->rfds, &this->wfds, NULL, NULL);
        if (this->numOfSocketSignaled == SOCKET_ERROR) { //select函数返回有可读或可写的socket的总数，保存在rtn里.最后一个参数设定等待时间，如为NULL则为阻塞模式
            cout << "select() failed with error!\n";
            return -1;
        }
        else if (this->numOfSocketSignaled == 0)//没有可读写或错误的文件
        {
            cout << "no file can read or write or error!\n";
            return 0;
        }

        //当程序运行到这里，意味着有用户连接请求到来，或有用户数据到来，或有会话socket可以发送数据

        //首先检查是否有客户请求连接到来
        if (this->AcceptRequestionFromClient() != 0) {
            cout << "no client request!\n";
            return -1;
        }

        //然后向客户端发送数据
        //this->ForwardMessage();

        //最后接受客户端发来的数据
        this->ReceieveMessageFromClients();
        //time(&(this->time_end));
        //if (this->time_end - this->time_begin > 15)//超时，设定为15秒
        //    return 0;
        //cout << "loop sleep" << endl;
        //睡眠500毫秒,把CPU让出来
        //让当前正在执行的线程休眠（暂停执行），而且在睡眠的过程是不释放资源的，保持着锁
        //Sleep(500);
    }

    return 0;
}

void Server::fileProcess(SOCKET socket, char * recvBuf)
{
    cout << "process begin!" << endl;
    //该函数已经接收到来自socket的消息，进行处理并发送出去
    string data(recvBuf);
    //检查报文头的GET和路径
    if (data.find("GET") != 0) //报文头不是GET
    {
        cout << "head is not GET! It is : " << endl;
        cout << data.substr(0, data.find(' ')) << endl;
        this->error(socket,true);
        return;
    }
    //temp存储文件名
    string temp = this->Filetable + data.substr(data.find('/') + 1, data.find(' ', data.find('/')) - data.find('/') - 1);
    const char * filename = temp.c_str();
    FILE * p_f = fopen(filename, "rb");
    if (p_f == NULL)
    {
        //文件名错误
        //QMessageBox::warning(this,"ERROR","can not find this file!",QMessageBox::Ok);
        cout << "can not find this file!" << endl ;
        this->error(socket,false);
        return;
    }
    //获取文件大小
    struct stat temp_stat;
    if (stat(filename, &temp_stat) == -1) return;//获取文件信息错误
    unsigned long filesize = temp_stat.st_size;
    //新开辟一块临时存储空间存放要读取的文件
    char * fileBuf = new char[filesize];
    //可以读取html文件，txt文件
    memset(fileBuf, 0, filesize);
    fread(fileBuf, 1, filesize, p_f);
    fclose(p_f);
    //temp中存储类型后缀
    temp = temp.substr(temp.find('.') + 1, temp.size() - temp.find('.') - 1);
    //headBuf中存报文头部
    char * headBuf = new char[256];
    memset(headBuf, 0, 256);
    if (temp == "txt" || temp == "html")
        sprintf(headBuf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %lu\r\n\r\n", filesize);
    else if (temp == "png")
        sprintf(headBuf, "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: %lu\r\n\r\n", filesize);
    if (send(socket, headBuf, strlen(headBuf), 0) == SOCKET_ERROR)
        cout << "head send fail!" << endl;
    else
        cout << "head send success!" << endl;
    if (send(socket, fileBuf, filesize, 0) == SOCKET_ERROR)
        cout << "content send fail!" << endl;
    else
        cout << "content send success!" << endl;

    //cout << "content send success! " << endl;
    memset(headBuf, 0, 256);
    memset(fileBuf, 0, filesize);
    delete[]headBuf;
    delete[]fileBuf;
    cout << "process end!" << endl;
}

void Server::print(SOCKET socket, char * recvBuf)
{
    cout << "print IP, port, get line:" << endl;
    string clientAddress;
    sockaddr_in clientAddr;
    int nameLen = sizeof(clientAddr);
    if(getsockname(socket, (LPSOCKADDR)&clientAddr, &nameLen) != SOCKET_ERROR)
        clientAddress += inet_ntoa(clientAddr.sin_addr);//以点分十进制形式返回IP地址
    if(clientAddress.empty()) //
    {
        cout << "get socket IP fail!" << endl;
        return;
    }
    cout << "IP: " << clientAddress << endl;
    string temp(clientAddress);
    MainWindow::out_ip = QString::fromStdString(temp);
    qDebug() << "port: " << htons(clientAddr.sin_port) << endl;
    //输出GET请求
    MainWindow::out_port = QString::fromStdString(std::to_string(htons(clientAddr.sin_port)));
    string get = recvBuf;
    //get = get.substr(0, get.find("HTTP"));
    MainWindow::out_head = QString::fromStdString(get);
    cout << get << endl;
    cout << "print end" << endl;
    emit sig(MainWindow::out_ip,MainWindow::out_port,MainWindow::out_head);
}
void Server::error(SOCKET socket, bool flag)
{
    string s("");
    if(flag) s+="D:/noget.html";//报文头部不是GET
    else s+="D:/not.html";//文件不存在
    const char * filename = s.c_str();
    FILE * p_f = fopen(filename, "rb");
    struct stat temp_stat;
    stat(filename, &temp_stat);
    unsigned long filesize = temp_stat.st_size;
    //新开辟一块临时存储空间存放要读取的文件
    char * fileBuf = new char[filesize];
    //可以读取html文件，txt文件
    memset(fileBuf,'\0', filesize);
    fread(fileBuf, 1, filesize, p_f);
    fclose(p_f);
    string head("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ");
    head = head + std::to_string(filesize) + "\r\n\r\n";
    char * headbuf = new char[head.size()];
    memset(headbuf, '\0', head.size());
    strcpy(headbuf, head.c_str());
    if (send(socket, headbuf, head.size(), 0) == SOCKET_ERROR)
        cout << "head send fail!" << endl;
    else
        cout << "head send success!" << endl;
    if (send(socket, fileBuf, filesize, 0) == SOCKET_ERROR)
        cout << "file send fail!" << endl;
    else
        cout << "file send success!" << endl;
    memset(headbuf, '\0', head.size());
    memset(fileBuf, '\0', filesize);
    delete []headbuf;
    delete []fileBuf;
    return ;
}
