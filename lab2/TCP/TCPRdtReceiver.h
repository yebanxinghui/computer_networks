#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
class TCPRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���

public:
	TCPRdtReceiver();
	virtual ~TCPRdtReceiver();

public:

	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
};



