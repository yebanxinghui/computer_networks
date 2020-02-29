#include "TCPRdtSender.h"
#include <fstream>

TCPRdtSender::TCPRdtSender() :base(0), next(0), window(4), waitingState(false),times(0),lastack(-1)
{//����ʱ�����������һ����ž�Ϊ0�����ڴ�СΪ4������Ϊ��
}
TCPRdtSender::~TCPRdtSender()
{
}
bool TCPRdtSender::getWaitingState() {//���ش�����״̬
	return waitingState;//false��ʾ����δ����true��ʾ��������
}


bool TCPRdtSender::send(const Message &message) {
	if (this->getWaitingState()) {
		return false;
	}
	this->next = (this->next + 1) % 8;//ģ8�ӷ���ʹ��next��Χ��0-7��
	if ((this->next + 8 - this->base) % 8 == 4)
		this->waitingState = true;//�������ˣ����õȴ�״̬

	Packet *pac = new Packet;
	pac->acknum = -1;//���Ը��ֶ�
	pac->seqnum = (this->next + 7) % 8;//
	pac->checksum = 0;//У���Ϊ0
	memcpy(pac->payload, message.data, sizeof(message.data));
	pac->checksum = pUtils->calculateCheckSum(*pac);
	pUtils->printPacket("�����ӡ��TCPRdt���͵ı���", *pac);//��Message�е���Ϣ��װ��Packet��,�����

	this->packetWaitingAck.push_back(*pac);//���±�����ȴ�ackӦ����ı��Ķ���β
	if (this->next == (base + 1) % 8)//����Packet�Ƕ����ײ���ʱ��Ž��м�ʱ������������ʱ����ʱ
		pns->startTimer(SENDER, Configuration::TIME_OUT, pac->seqnum);	//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, *pac);//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�																				//����ȴ�״̬
	delete pac;
	return true;
}

void TCPRdtSender::receive(const Packet &ackPkt) {//����ȷ��Ack������NetworkServiceSimulator����
	if (this->packetWaitingAck.empty())//����ȴ�ack����Ϊ�գ�ʲô������
		return;
	if (pUtils->calculateCheckSum(ackPkt) != ackPkt.checksum)//���У����Ƿ���ȷ
	{
		pUtils->printPacket("���ͷ��յ�ackУ�������", ackPkt);
		return;
	}
	pUtils->printPacket("���ͷ��յ�ackУ������ȷ", ackPkt);
	if (ackPkt.acknum != lastack)//����յ���ack��ź���һ����һ��
	{
		this->lastack = ackPkt.acknum;
		this->times = 1;
	}
	else this->times++;//����յ����ظ���ack���
	ofstream  ofresult("result.txt ", ios::app);
	for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
		ofresult << it->seqnum << ' ';
	ofresult << endl;
	ofresult.close();
	if (this->times == 3)//�����������ظ�ack������ش�����û�յ����Ǹ�����
	{
		for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
			pUtils->printPacket("��ӡ�����ش�֮ǰ�Ĵ�������ı���", *it);
		pUtils->printPacket("�����ش��ȴ����е�һ������", *(this->packetWaitingAck.begin()));
		pns->stopTimer(SENDER, (*(this->packetWaitingAck.begin())).seqnum);//�ȹرշ��ͷ���ʱ��
		pns->startTimer(SENDER, Configuration::TIME_OUT, (this->packetWaitingAck.begin())->seqnum);	//�������ͷ���ʱ��
		pns->sendToNetworkLayer(RECEIVER, *(this->packetWaitingAck.begin()));
		return;
	}	
	bool ackIn = false;//��ʾ���յ���ack�Ƿ��Ӧ�ȴ�ack�������ĳ������
	vector<Packet>::iterator it;
	for (it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
	{
		if (it->seqnum == ackPkt.acknum)
		{
			ackIn = true;
			break;
		}
	}
	if (it != this->packetWaitingAck.end()) it++;
	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (ackIn) {
		if (this->getWaitingState()) this->waitingState = false;
		pns->stopTimer(SENDER, (this->packetWaitingAck.begin())->seqnum);//�رն�ʱ��
		this->packetWaitingAck.erase(this->packetWaitingAck.begin(), it);//�ӵȴ�������ɾȥ�յ�ack�ı��İ���������ǰ�ı���
		this->base = (ackPkt.acknum + 1) % 8;			//��һ�����������0-7֮���л�
		if (!this->packetWaitingAck.empty())			//�����Ϊ�յ�Ack���ĵı��ĵ���ŵ���һ�����
			pns->startTimer(SENDER, Configuration::TIME_OUT, (this->packetWaitingAck.begin())->seqnum);	//�����������ͷ���ʱ��
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
	}
	else pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ�ϣ��ط��ϴη��͵ı���", ackPkt);
	cout << "�������" << endl;
}

void TCPRdtSender::timeoutHandler(int seqNum)
{
	//Timeout handler������NetworkServiceSimulator����
	pUtils->printPacket("��ʱ���ط��ȴ����е�һ������", *(this->packetWaitingAck.begin()));
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, *(this->packetWaitingAck.begin()));//���·��͵ȴ����е�һ������

}
