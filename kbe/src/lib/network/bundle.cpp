#include "bundle.hpp"
#include "network/network_interface.hpp"
#include "network/packet.hpp"
#include "network/channel.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"

#ifndef CODE_INLINE
#include "bundle.ipp"
#endif

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
Bundle::Bundle(Channel * pChannel, PacketType pt):
	pChannel_(pChannel),
	pCurrPacket_(NULL),
	currMsgID_(0),
	currMsgPacketCount_(0),
	currMsgLength_(0),
	currMsgHandlerLength_(0),
	packets_(),
	isTCPPacket_(pt == TCP_PACKET)
{
	 newPacket();
}

//-------------------------------------------------------------------------------------
Bundle::~Bundle()
{
	clear();
	SAFE_RELEASE(pCurrPacket_);
}

//-------------------------------------------------------------------------------------
Packet* Bundle::newPacket()
{
	if(isTCPPacket_ == TCP_PACKET)
		pCurrPacket_ = new TCPPacket;
	else
		pCurrPacket_ = new UDPPacket;
	
	return pCurrPacket_;
}

//-------------------------------------------------------------------------------------
void Bundle::finish(void)
{
	currMsgLength_ += pCurrPacket_->totalSize();
	packets_.push_back(pCurrPacket_);

	// 此处对于非固定长度的消息来说需要设置它的最终长度信息
	if(currMsgHandlerLength_ < 0)
		packets_[packets_.size() - currMsgPacketCount_]->setPacketLength(currMsgLength_);

	pCurrPacket_ = NULL;
	currMsgID_ = 0;
	currMsgPacketCount_ = 0;
	currMsgLength_ = 0;
}

//-------------------------------------------------------------------------------------
void Bundle::clear()
{
	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
		delete (*iter);
	
	packets_.clear();
	SAFE_RELEASE(pCurrPacket_);

	currMsgID_ = 0;
	currMsgPacketCount_ = 0;
}

//-------------------------------------------------------------------------------------
void Bundle::send(NetworkInterface & networkInterface, Channel * pChannel)
{
	finish();
	networkInterface.send(*this, pChannel);
}

//-------------------------------------------------------------------------------------
void Bundle::send(EndPoint& ep)
{
	finish();

	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
	{
		Packet* pPacket = (*iter);
		ep.send(pPacket->data(), pPacket->totalSize());
		delete pPacket;
	}
	
	packets_.clear();
}

//-------------------------------------------------------------------------------------
void Bundle::newMessage(const MessageHandler& msgHandler)
{
	KBE_ASSERT(pCurrPacket_ != NULL);

	pCurrPacket_->messageID(msgHandler.msgID);

	numMessages_++;
	currMsgID_ = msgHandler.msgID;
	currMsgPacketCount_ = 1;
	currMsgHandlerLength_ = msgHandler.msgLen;
}

//-------------------------------------------------------------------------------------
}
}