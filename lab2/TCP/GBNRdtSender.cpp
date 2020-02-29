#include "GBNRdtSender.h"


GBNRdtSender::GBNRdtSender():base(0),next(0),window(4),waitingState(false)
{//����ʱ�����������һ����ž�Ϊ0�����ڴ�СΪ4������Ϊ��
}
GBNRdtSender::~GBNRdtSender()
{
}
bool GBNRdtSender::getWaitingState() {//���ش�����״̬
	return waitingState;//false��ʾ����δ����true��ʾ��������
}


bool GBNRdtSender::send(const Message &message) {
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
	pUtils->printPacket("�����ӡ��GBNRdt���͵ı���",*pac);//��Message�е���Ϣ��װ��Packet��,�����

	this->packetWaitingAck.push_back(*pac);//���±�����ȴ�ackӦ����ı��Ķ���β
	if (this->next == (base + 1) % 8)//����Packet�Ƕ����ײ���ʱ��Ž��м�ʱ������������ʱ����ʱ
		pns->startTimer(SENDER, Configuration::TIME_OUT,pac->seqnum);	//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, *pac);//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�																				//����ȴ�״̬
	delete pac;
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt) {//����ȷ��Ack������NetworkServiceSimulator����
	if (this->packetWaitingAck.empty())//����ȴ�ack����Ϊ�գ�ʲô������
		return;
	int checkSum = pUtils->calculateCheckSum(ackPkt);//���У����Ƿ���ȷ
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
	if (checkSum == ackPkt.checksum && ackIn) {
		if (this->getWaitingState()) this->waitingState = false;
		pns->stopTimer(SENDER, (this->packetWaitingAck.begin())->seqnum);//�رն�ʱ��
		this->packetWaitingAck.erase(this->packetWaitingAck.begin(), it);//�ӵȴ�������ɾȥ�յ�ack�ı��İ���������ǰ�ı���
		this->base = (ackPkt.acknum+1)%8;			//��һ�����������0-7֮���л�
		if(!this->packetWaitingAck.empty())			//�����Ϊ�յ�Ack���ĵı��ĵ���ŵ���һ�����
			pns->startTimer(SENDER, Configuration::TIME_OUT, (this->packetWaitingAck.begin())->seqnum);	//�����������ͷ���ʱ��
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
	}
	else pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ�ϣ��ط��ϴη��͵ı���", ackPkt);
	cout << "�������" << endl;
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
	//Timeout handler������NetworkServiceSimulator����
	for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
		pUtils->printPacket("��ʱ���ط��ϴη��͵ı���", *it);
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT,seqNum);			//�����������ͷ���ʱ��
	for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
		pns->sendToNetworkLayer(RECEIVER, *it);							//���·���ÿ�����ݰ�

}
