#include <winsock2.h>
#include <list>
#include <string>
#include <map>
#include <thread>
#include <QObject>
#include <QThread>
using namespace std;
//服务器
/*
server类用于完成主要功能
*/
class Server: public QObject
{
    Q_OBJECT
    private:

        //char* recvBuf;		//接受缓冲区
        fd_set rfds;			//用于检查socket是否有数据到来的的文件描述符，用于socket非阻塞模式下等待网络事件通知（有数据到来）
        fd_set wfds;			//用于检查socket是否可以发送的文件描述符，用于socket非阻塞模式下等待网络事件通知（可以发送数据）
        sockaddr_in srvAddr;	//服务器端IP地址
        list<SOCKET> *sessions;				//当前的会话socket队列
        list<SOCKET> *closedSessions;		//所有已失效的会话socket队列
        //list<string> *rcvedMessages;		//已接收到的客户端信息队列
        int numOfSocketSignaled;			//可读、写、有请求到来的socket总数
        map<SOCKET, string> *clientAddrMaps;//客户端地址列表，地址以(key,value)对形式保存，key为SOCKET类型，value为string类型
        int port;				     //配置端口
        string ServerIP;			 //配置服务器IP
        string Filetable;            //配置主目录
        map<SOCKET, int> *socketLock;//多线程锁，值为0代表还未处理，1代表正在被某个线程处理
        /*time_t time_begin, time_end; //计时器，用于请求超时自动退出
        std::thread recvMessageThread(SOCKET socket)  //多线程处理
        {
            return thread(&Server::recvMessage, this, socket);
        }*/
    protected:

        //virtual void AddRecvMessage(string str);								//将收到的客户端消息保存到消息队列
        virtual void AddSession(SOCKET session);								//将新的会话socket加入队列
        virtual void AddClosedSession(SOCKET session);							//将失效的会话socket加入队列
        virtual void RemoveClosedSession(SOCKET closedSession);					//将失效的SOCKET从会话socket队列删除
        virtual void RemoveClosedSession();										//将失效的SOCKET从会话socket队列删除
        //virtual void ForwardMessage();										//向其他客户转发信息
        virtual void recvMessage(SOCKET s);										//从SOCKET s接受消息
        //virtual void sendMessage(SOCKET s, string msg);						//向SOCKET s发送消息
        virtual string  GetClientAddress(SOCKET s);								//得到客户端IP地址
        virtual string  GetClientAddress(map<SOCKET, string> *maps, SOCKET s);	//得到客户端IP地址
        virtual void  ReceieveMessageFromClients();								//接受客户端发来的信息
        virtual int AcceptRequestionFromClient();								//等待客户端连接请求
        void fileProcess(SOCKET s, char *recvBuf);								//处理文件并转发
        void error(SOCKET socket, bool flag);									//出现问题，flag=true表示头部不是GET，false即文件不存在
        void print(SOCKET s, char *recvBuf);                                    //输出IP, port, GET等信息
    public:
        SOCKET srvSocket;		//服务器socket
        explicit Server(QObject *parent = 0);
        virtual ~Server(void);
        virtual int WinsockStartup();	//初始化Winsock
        virtual int ServerStartup();	//初始化Server，包括创建SOCKET，绑定到IP和PORT
        virtual int ListenStartup();	//开始监听客户端请求
        virtual int Loop();				//循环执行 "等待客户端请求"->“向其他客户转发信息”->"等待客户端消息"
    signals:
        void sig(QString ip, QString port, QString head);
};
