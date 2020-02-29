// StopWait.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "TCPRdtSender.h"
#include "TCPRdtReceiver.h"


int main(int argc, char* argv[])
{
	RdtSender *ps = new TCPRdtSender();
	RdtReceiver * pr = new TCPRdtReceiver();
//	pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("E:\\cpp\\shiyan4\\input.txt");
	pns->setOutputFile("E:\\cpp\\shiyan4\\output.txt");
	pns->start();
	system("pause");
	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	return 0;
}

