#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
#include <vector>
class SRRdtSender :public RdtSender
{
private:
	int sendbase;							//发送基序号，最早的未确认分组的信号
	int next;								//下一个发送序号，最小的未使用的序号
	int window;								//滑动窗口的大小
	bool waitingState;						//表示窗口是否为满,满则为true
	vector<Packet> packetWaitingAck;		//已发送并等待Ack的数据包,用vector模拟一个分组序号队列

public:
	bool getWaitingState();
	bool send(const Message &message);		//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt);		//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);		//Timeout handler，将被NetworkServiceSimulator调用

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};


