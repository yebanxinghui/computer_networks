#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
#include <vector>
class TCPRdtSender :public RdtSender
{
private:
	int base;								//基序号，最早的未确认分组的信号
	int next;								//下一个发送序号，最小的未使用的序号
	int window;								//滑动窗口的大小
	bool waitingState;						//表示窗口是否为满,满则为true
	vector<Packet> packetWaitingAck;		//已发送并等待Ack的数据包,用vector模拟一个分组序号队列
	int times;								//接收到相同ack的次数，一旦重复三次即重传
	int lastack;							//上一次收到的ack序号
public:
	bool getWaitingState();
	bool send(const Message &message);		//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt);		//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);		//Timeout handler，将被NetworkServiceSimulator调用

public:
	TCPRdtSender();
	virtual ~TCPRdtSender();
};


