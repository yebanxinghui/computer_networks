#include "SRRdtSender.h"
#include <fstream>

SRRdtSender::SRRdtSender() :sendbase(0), next(0), window(4), waitingState(false)
{//构造时，基序号与下一个序号均为0，窗口大小为4，窗口为空
}
SRRdtSender::~SRRdtSender()
{
}
bool SRRdtSender::getWaitingState() {//返回窗口满状态
	return waitingState;//false表示窗口未满，true表示窗口已满
}


bool SRRdtSender::send(const Message &message) {
	if (this->getWaitingState()) {
		return false;
	}
	this->next = (this->next + 1) % 8;//模8加法，使得next范围在0-7里
	if ((this->next + 8 - this->sendbase) % 8 == 4)
		this->waitingState = true;//窗口满了，设置等待状态

	Packet *pac = new Packet;
	pac->acknum = -1;//忽略该字段
	pac->seqnum = (this->next + 7) % 8;
	pac->checksum = 0;//校验和为0
	memcpy(pac->payload, message.data, sizeof(message.data));
	pac->checksum = pUtils->calculateCheckSum(*pac);
	pUtils->printPacket("下面打印出SRRdt发送的报文", *pac);//将Message中的消息封装到Packet中,并输出

	this->packetWaitingAck.push_back(*pac);//将新报文入等待ack应答包的报文队列尾
	//这个地方就和GBN不一样了，这个是只要成功发送报文出去了就开始计时
	pns->startTimer(SENDER, Configuration::TIME_OUT, pac->seqnum);	//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, *pac);//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方																				//进入等待状态
	delete pac;
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt) {//接受确认Ack，将被NetworkServiceSimulator调用
	if (this->packetWaitingAck.empty())//如果等待ack队列为空，什么都不做
		return;
	if (pUtils->calculateCheckSum(ackPkt) != ackPkt.checksum) {//检查校验和是否正确
		pUtils->printPacket("发送方没有正确收到确认，重发上次发送的报文", ackPkt);
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
		if (it->seqnum == ackPkt.acknum)//如果ack对应了等待队列里的某一个报文
		{
			it->acknum = ackPkt.acknum;//将待确认队列中的该报文的acknum由-1改成seqnum
			pns->stopTimer(SENDER, ackPkt.acknum);//停掉该报文计时器
			if (it == this->packetWaitingAck.begin())//如果收到的报文是等待队列第一个
			{
				if (this->getWaitingState()) this->waitingState = false;
				while (!this->packetWaitingAck.empty() && (this->packetWaitingAck.begin())->acknum != -1)
				{
					pUtils->printPacket("删除已收到Ack报文的报文", *(this->packetWaitingAck.begin()));
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
	//Timeout handler，将被NetworkServiceSimulator调用
	for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
	{	//这个和GBN不一样，GBN是只要超时，全部重发，这个只要重发超时的那个就行
		if (it->seqnum == seqNum) {
			pUtils->printPacket("超时，重发上次发送的报文", *it);
			pns->sendToNetworkLayer(RECEIVER, *it);	//重新发送每个数据包
		}
	}
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
}
