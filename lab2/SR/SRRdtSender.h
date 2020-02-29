#pragma once
#include "stdafx.h"
#include "Global.h"
#include "DataStructure.h"
#include <vector>
class SRRdtSender :public RdtSender
{
private:
	int sendbase;							//���ͻ���ţ������δȷ�Ϸ�����ź�
	int next;								//��һ��������ţ���С��δʹ�õ����
	int window;								//�������ڵĴ�С
	bool waitingState;						//��ʾ�����Ƿ�Ϊ��,����Ϊtrue
	vector<Packet> packetWaitingAck;		//�ѷ��Ͳ��ȴ�Ack�����ݰ�,��vectorģ��һ��������Ŷ���

public:
	bool getWaitingState();
	bool send(const Message &message);		//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet &ackPkt);		//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);		//Timeout handler������NetworkServiceSimulator����

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};


