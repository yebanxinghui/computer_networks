#include "TCPRdtSender.h"
#include <fstream>

TCPRdtSender::TCPRdtSender() :base(0), next(0), window(4), waitingState(false),times(0),lastack(-1)
{//构造时，基序号与下一个序号均为0，窗口大小为4，窗口为空
}
TCPRdtSender::~TCPRdtSender()
{
}
bool TCPRdtSender::getWaitingState() {//返回窗口满状态
	return waitingState;//false表示窗口未满，true表示窗口已满
}


bool TCPRdtSender::send(const Message &message) {
	if (this->getWaitingState()) {
		return false;
	}
	this->next = (this->next + 1) % 8;//模8加法，使得next范围在0-7里
	if ((this->next + 8 - this->base) % 8 == 4)
		this->waitingState = true;//窗口满了，设置等待状态

	Packet *pac = new Packet;
	pac->acknum = -1;//忽略该字段
	pac->seqnum = (this->next + 7) % 8;//
	pac->checksum = 0;//校验和为0
	memcpy(pac->payload, message.data, sizeof(message.data));
	pac->checksum = pUtils->calculateCheckSum(*pac);
	pUtils->printPacket("下面打印出TCPRdt发送的报文", *pac);//将Message中的消息封装到Packet中,并输出

	this->packetWaitingAck.push_back(*pac);//将新报文入等待ack应答包的报文队列尾
	if (this->next == (base + 1) % 8)//当该Packet是队列首部的时候才进行计时操作，后面暂时不计时
		pns->startTimer(SENDER, Configuration::TIME_OUT, pac->seqnum);	//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, *pac);//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方																				//进入等待状态
	delete pac;
	return true;
}

void TCPRdtSender::receive(const Packet &ackPkt) {//接受确认Ack，将被NetworkServiceSimulator调用
	if (this->packetWaitingAck.empty())//如果等待ack队列为空，什么都不做
		return;
	if (pUtils->calculateCheckSum(ackPkt) != ackPkt.checksum)//检查校验和是否正确
	{
		pUtils->printPacket("发送方收到ack校验码错误", ackPkt);
		return;
	}
	pUtils->printPacket("发送方收到ack校验码正确", ackPkt);
	if (ackPkt.acknum != lastack)//如果收到的ack序号和上一个不一样
	{
		this->lastack = ackPkt.acknum;
		this->times = 1;
	}
	else this->times++;//如果收到的重复的ack序号
	ofstream  ofresult("result.txt ", ios::app);
	for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
		ofresult << it->seqnum << ' ';
	ofresult << endl;
	ofresult.close();
	if (this->times == 3)//若出现三次重复ack则快速重传最早没收到的那个报文
	{
		for (vector<Packet>::iterator it = this->packetWaitingAck.begin(); it != this->packetWaitingAck.end(); it++)
			pUtils->printPacket("打印快速重传之前的窗口里面的报文", *it);
		pUtils->printPacket("快速重传等待队列第一个报文", *(this->packetWaitingAck.begin()));
		pns->stopTimer(SENDER, (*(this->packetWaitingAck.begin())).seqnum);//先关闭发送方定时器
		pns->startTimer(SENDER, Configuration::TIME_OUT, (this->packetWaitingAck.begin())->seqnum);	//启动发送方定时器
		pns->sendToNetworkLayer(RECEIVER, *(this->packetWaitingAck.begin()));
		return;
	}	
	bool ackIn = false;//表示接收到的ack是否对应等待ack队列里的某个报文
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
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (ackIn) {
		if (this->getWaitingState()) this->waitingState = false;
		pns->stopTimer(SENDER, (this->packetWaitingAck.begin())->seqnum);//关闭定时器
		this->packetWaitingAck.erase(this->packetWaitingAck.begin(), it);//从等待队列中删去收到ack的报文包括自身以前的报文
		this->base = (ackPkt.acknum + 1) % 8;			//下一个发送序号在0-7之间切换
		if (!this->packetWaitingAck.empty())			//基序号为收到Ack报文的报文的序号的下一个序号
			pns->startTimer(SENDER, Configuration::TIME_OUT, (this->packetWaitingAck.begin())->seqnum);	//重新启动发送方定时器
		pUtils->printPacket("发送方正确收到确认", ackPkt);
	}
	else pUtils->printPacket("发送方没有正确收到确认，重发上次发送的报文", ackPkt);
	cout << "接收完成" << endl;
}

void TCPRdtSender::timeoutHandler(int seqNum)
{
	//Timeout handler，将被NetworkServiceSimulator调用
	pUtils->printPacket("超时，重发等待队列第一个报文", *(this->packetWaitingAck.begin()));
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, *(this->packetWaitingAck.begin()));//重新发送等待队列第一个报文

}
