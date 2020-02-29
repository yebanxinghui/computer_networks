#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
#include <vector>
class SRRdtReceiver :public RdtReceiver
{
private:
	int recbase;					//���ջ����
	int window;						//�������ڵĴ�С
	Packet ack;						//ȷ��ack����
	vector<Packet> lastAckPkt;		//���յķ���������

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	bool inwindow(int a, int b, int n); //�ж�a,b������Ƿ���n����
	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
};



