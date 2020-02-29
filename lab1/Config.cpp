#include "Config.h"
#include <string>

using namespace std;

Config::Config(void)
{
}

Config::~Config(void)
{
}

const string Config::SERVERADDRESS = "127.0.0.1";	//服务器IP地址
const int Config::MAXCONNECTION = 50;				//最大连接数5
const int Config::BUFFERLENGTH = 65536;				//缓冲区大小65536字节
const int Config::PORT = 80;						//服务器端口80
const u_long Config::BLOCKMODE = 1;					//SOCKET为非阻塞模式
