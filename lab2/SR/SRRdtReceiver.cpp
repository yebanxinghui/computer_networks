#include "SRRdtReceiver.h"
#include <algorithm>
#include <fstream>
SRRdtReceiver::SRRdtReceiver() :recbase(0), window(4)
{
	ack.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	ack.checksum = 0;
	ack.seqnum = -1;	//忽略该字段
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
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确
	if (checkSum != packet.checksum) {
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		return;
	}
	if (this->inwindow(this->recbase, packet.seqnum, this->window))//如果接收到的序号在窗口大小内
	{
		this->ack.acknum = packet.seqnum;//发回一个ack包
		this->ack.checksum = pUtils->calculateCheckSum(this->ack);
		pUtils->printPacket("接收方发送ACK报文", this->ack);
		pns->sendToNetworkLayer(SENDER, this->ack);//递交给网络层送回去
 
		bool pktIn = false;//记录新收到的packet被缓存了没
		for (vector<Packet>::iterator it = this->lastAckPkt.begin(); it != lastAckPkt.end(); it++)
		{
			if (it->seqnum == packet.seqnum)//如果这个包在接收队列里面找到了
			{
				pktIn = true;
				break;
			}
		}
		if (!pktIn)//没找到就缓存呗
		{
			//将这个报文放在缓存队列里相对位置
			int index = (packet.seqnum + 8 - this->recbase) % 8;
			vector<Packet>::iterator it = this->lastAckPkt.begin() + index;
			*it = packet;
			ofstream  ofresult("result2.txt", ios::app);
			for (vector<Packet>::iterator it = this->lastAckPkt.begin(); it != this->lastAckPkt.end(); it++)
				ofresult << it->seqnum << ' ';
			ofresult << endl;
			ofresult.close();
			if (packet.seqnum == this->recbase)//一旦收到了缓存队列的第一个缺失报文，就可以弹出连续报文了
			{
				//使用nullpkt用作空报文占位
				Packet nullpkt;
				nullpkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
				nullpkt.checksum = 0;
				nullpkt.seqnum = -1;//忽略该字段
				for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
					nullpkt.payload[i] = '.';
				nullpkt.checksum = pUtils->calculateCheckSum(nullpkt);
				while ((this->lastAckPkt.begin())->seqnum != -1)
				{
					//取出Message，向上递交给应用层
					Message msg;
					memcpy(msg.data, (this->lastAckPkt.begin())->payload, sizeof((this->lastAckPkt.begin())->payload));
					pns->delivertoAppLayer(RECEIVER, msg);
					this->recbase = (this->recbase + 1) % 8;
					this->lastAckPkt.erase(this->lastAckPkt.begin());
					this->lastAckPkt.push_back(nullpkt);
				}
			}
			else pUtils->printPacket("该报文已经被缓存过了", packet);
		}
	}
	else {//刚好不在窗口内,说明是之前已接受到的报文，可能由于某些原因发送方没有收到ack所以重发了一遍
		this->ack.acknum = packet.seqnum;
		this->ack.checksum = pUtils->calculateCheckSum(this->ack);
		pUtils->printPacket("接收方发送确认报文", ack);
		pns->sendToNetworkLayer(SENDER, this->ack);
	}
}