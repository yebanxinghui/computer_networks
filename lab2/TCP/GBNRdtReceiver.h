#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
class GBNRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���

public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

public:

	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
};


