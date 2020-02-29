#include "SRRdtSender.h"
#include <fstream>

SRRdtSender::SRRdtSender() :sendbase(0), next(0), window(4), waitingState(false)
{//����ʱ�����������һ����ž�Ϊ0�����ڴ�СΪ4������Ϊ��
}
SRRdtSender::~SRRdtSender()
{
}
bool SRRdtSender::getWaitingState() {//���ش�����״̬
	return waitingState;//false��ʾ����δ����true��ʾ��������
}


bool SRRdtSender::send(const Message &message) {
	if (this->getWaitingState()) {
		return false;
	}
	this->next = (this->next + 1) % 8;//ģ8�ӷ���ʹ��next��Χ��0-7��
	if ((this->next + 8 - this->sendbase) % 8 == 4)
		this->waitingState = true;//�������ˣ����õȴ�״̬

	Packet *pac = new Packet;
	pac->acknum = -1;//���Ը��ֶ�
	pac->seqnum = (this->next + 7) % 8;
	pac->checksum = 0;//У���Ϊ0
	memcpy(pac->payload, message.data, sizeof(message.data));
	pac->checksum = pUtils->calculateCheckSum(*pac);
	pUtils->printPacket("�����ӡ��SRRdt���͵ı���", *pac);//��Message�е���Ϣ��װ��Packet��,�����

	this->packetWaitingAck.push_back(*pac);//���±�����ȴ�ackӦ����ı��Ķ���β
	//����ط��ͺ�GBN��һ���ˣ������ֻҪ�ɹ����ͱ��ĳ�ȥ�˾Ϳ�ʼ��ʱ
	pns->startTimer(SENDER, Configuration::TIME_OUT, pac->seqnum);	//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, *pac);//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�																				//����ȴ�״̬
	delete pac;
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt) {//����ȷ��Ack������NetworkServiceSimulator����
	if (this->packetWaitingAck.empty())//����ȴ�ack����Ϊ�գ�ʲô������
		return;
	if (pUtils->calculateCheckSum(ackPkt) != ackPkt.checksum) {//���У����Ƿ���ȷ
		pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ�ϣ��ط��ϴη��͵ı���", ackPkt);
		return;
	}
	ofstream  ofresult("result.txt ", ios::app);
	for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
		ofresult << it->seqnum << ' ';
	ofresult << endl;
	ofresult.close();
	vector<Packet>::iterator it;
	for (it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
	{	
		if (it->seqnum == ackPkt.acknum)//���ack��Ӧ�˵ȴ��������ĳһ������
		{
			it->acknum = ackPkt.acknum;//����ȷ�϶����еĸñ��ĵ�acknum��-1�ĳ�seqnum
			pns->stopTimer(SENDER, ackPkt.acknum);//ͣ���ñ��ļ�ʱ��
			if (it == this->packetWaitingAck.begin())//����յ��ı����ǵȴ����е�һ��
			{
				if (this->getWaitingState()) this->waitingState = false;
				while (!this->packetWaitingAck.empty() && (this->packetWaitingAck.begin())->acknum != -1)
				{
					pUtils->printPacket("ɾ�����յ�Ack���ĵı���", *(this->packetWaitingAck.begin()));
					this->packetWaitingAck.erase(this->packetWaitingAck.begin());
					this->sendbase = (this->sendbase + 1) % 8;
				}
			}
			break;
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum)
{
	//Timeout handler������NetworkServiceSimulator����
	for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
	{	//�����GBN��һ����GBN��ֻҪ��ʱ��ȫ���ط������ֻҪ�ط���ʱ���Ǹ�����
		if (it->seqnum == seqNum) {
			pUtils->printPacket("��ʱ���ط��ϴη��͵ı���", *it);
			pns->sendToNetworkLayer(RECEIVER, *it);	//���·���ÿ�����ݰ�
		}
	}
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
}
