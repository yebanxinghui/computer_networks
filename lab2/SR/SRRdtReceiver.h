#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
#include <vector>
class SRRdtReceiver :public RdtReceiver
{
private:
	int recbase;					//接收基序号
	int window;						//滑动窗口的大小
	Packet ack;						//确认ack报文
	vector<Packet> lastAckPkt;		//接收的非连续报文

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	bool inwindow(int a, int b, int n); //判断a,b正间距是否在n以内
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
};



