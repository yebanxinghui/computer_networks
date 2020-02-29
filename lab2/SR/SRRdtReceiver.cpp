#include "SRRdtReceiver.h"
#include <algorithm>
#include <fstream>
SRRdtReceiver::SRRdtReceiver() :recbase(0), window(4)
{
	ack.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	ack.checksum = 0;
	ack.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		ack.payload[i] = '.';
	}
	ack.checksum = pUtils->calculateCheckSum(ack);
	Packet nullpkt = this->ack;
	for (int i = 0; i < 4; i++)
		this->lastAckPkt.push_back(nullpkt);
}


SRRdtReceiver::~SRRdtReceiver()
{
}
bool SRRdtReceiver::inwindow(int a, int b, int n)
{
	while (n--)
	{
		if (a == b) return true;
		a = (a + 1) % 8;
	}
	return false;
}
void SRRdtReceiver::receive(const Packet &packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ
	if (checkSum != packet.checksum) {
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		return;
	}
	if (this->inwindow(this->recbase, packet.seqnum, this->window))//������յ�������ڴ��ڴ�С��
	{
		this->ack.acknum = packet.seqnum;//����һ��ack��
		this->ack.checksum = pUtils->calculateCheckSum(this->ack);
		pUtils->printPacket("���շ�����ACK����", this->ack);
		pns->sendToNetworkLayer(SENDER, this->ack);//�ݽ���������ͻ�ȥ
 
		bool pktIn = false;//��¼���յ���packet��������û
		for (vector<Packet>::iterator it = this->lastAckPkt.begin(); it != lastAckPkt.end(); it++)
		{
			if (it->seqnum == packet.seqnum)//���������ڽ��ն��������ҵ���
			{
				pktIn = true;
				break;
			}
		}
		if (!pktIn)//û�ҵ��ͻ�����
		{
			//��������ķ��ڻ�����������λ��
			int index = (packet.seqnum + 8 - this->recbase) % 8;
			vector<Packet>::iterator it = this->lastAckPkt.begin() + index;
			*it = packet;
			ofstream  ofresult("result2.txt", ios::app);
			for (vector<Packet>::iterator it = this->lastAckPkt.begin(); it != this->lastAckPkt.end(); it++)
				ofresult << it->seqnum << ' ';
			ofresult << endl;
			ofresult.close();
			if (packet.seqnum == this->recbase)//һ���յ��˻�����еĵ�һ��ȱʧ���ģ��Ϳ��Ե�������������
			{
				//ʹ��nullpkt�����ձ���ռλ
				Packet nullpkt;
				nullpkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
				nullpkt.checksum = 0;
				nullpkt.seqnum = -1;//���Ը��ֶ�
				for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
					nullpkt.payload[i] = '.';
				nullpkt.checksum = pUtils->calculateCheckSum(nullpkt);
				while ((this->lastAckPkt.begin())->seqnum != -1)
				{
					//ȡ��Message�����ϵݽ���Ӧ�ò�
					Message msg;
					memcpy(msg.data, (this->lastAckPkt.begin())->payload, sizeof((this->lastAckPkt.begin())->payload));
					pns->delivertoAppLayer(RECEIVER, msg);
					this->recbase = (this->recbase + 1) % 8;
					this->lastAckPkt.erase(this->lastAckPkt.begin());
					this->lastAckPkt.push_back(nullpkt);
				}
			}
			else pUtils->printPacket("�ñ����Ѿ����������", packet);
		}
	}
	else {//�պò��ڴ�����,˵����֮ǰ�ѽ��ܵ��ı��ģ���������ĳЩԭ���ͷ�û���յ�ack�����ط���һ��
		this->ack.acknum = packet.seqnum;
		this->ack.checksum = pUtils->calculateCheckSum(this->ack);
		pUtils->printPacket("���շ�����ȷ�ϱ���", ack);
		pns->sendToNetworkLayer(SENDER, this->ack);
	}
}