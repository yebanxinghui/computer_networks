#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
class GBNRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	Packet lastAckPkt;				//上次发送的确认报文

public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

public:

	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
};



