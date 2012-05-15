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
Bundle::Bundle(Channel * pChannel, ProtocolType pt):
	pChannel_(pChannel),
	numMessages_(0),
	pCurrPacket_(NULL),
	currMsgID_(0),
	currMsgPacketCount_(0),
	currMsgLength_(0),
	currMsgHandlerLength_(0),
	currMsgLengthPos_(0),
	packets_(),
	isTCPPacket_(pt == PROTOCOL_TCP)
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
	if(isTCPPacket_)
		pCurrPacket_ = new TCPPacket;
	else
		pCurrPacket_ = new UDPPacket;
	
	return pCurrPacket_;
}

//-------------------------------------------------------------------------------------
void Bundle::finish(bool issend)
{
	if(issend)
	{
		currMsgPacketCount_++;
		packets_.push_back(pCurrPacket_);
	}

	currMsgLength_ += pCurrPacket_->totalSize();

	// 此处对于非固定长度的消息来说需要设置它的最终长度信息
	if(currMsgHandlerLength_ < 0)
	{
		Packet* pPacket = packets_[packets_.size() - currMsgPacketCount_];

		currMsgLength_ -= MERCURY_MESSAGE_ID_SIZE;
		currMsgLength_ -= MERCURY_MESSAGE_LENGTH_SIZE;

		memcpy(&pPacket->data()[currMsgLengthPos_], 
			(uint8*)&currMsgLength_, MERCURY_MESSAGE_LENGTH_SIZE);

	}
	
	if(issend)
	{
		currMsgHandlerLength_ = 0;
		pCurrPacket_ = NULL;
	}

	currMsgID_ = 0;
	currMsgPacketCount_ = 0;
	currMsgLength_ = 0;
	currMsgLengthPos_ = 0;
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
	currMsgLength_ = 0;
	currMsgLengthPos_ = 0;
	currMsgHandlerLength_ = 0;
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
	
	onSendComplete();
}

//-------------------------------------------------------------------------------------
void Bundle::sendto(EndPoint& ep, u_int16_t networkPort, u_int32_t networkAddr)
{
	finish();

	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
	{
		Packet* pPacket = (*iter);
		ep.sendto(pPacket->data(), pPacket->totalSize(), networkPort, networkAddr);
		delete pPacket;
	}
	
	onSendComplete();
}

//-------------------------------------------------------------------------------------
void Bundle::onSendComplete()
{
	packets_.clear();
}

//-------------------------------------------------------------------------------------
void Bundle::newMessage(const MessageHandler& msgHandler)
{
	finish(false);
	KBE_ASSERT(pCurrPacket_ != NULL);
	
	(*this) << msgHandler.msgID;
	pCurrPacket_->messageID(msgHandler.msgID);

	// 此处对于非固定长度的消息来说需要先设置它的消息长度位为0， 到最后需要填充长度
	if(msgHandler.msgLen == MERCURY_VARIABLE_MESSAGE)
	{
		MessageLength msglen = 0;
		currMsgLengthPos_ = pCurrPacket_->wpos();
		(*this) << msglen;
	}

	numMessages_++;
	currMsgID_ = msgHandler.msgID;
	currMsgPacketCount_ = 0;
	currMsgHandlerLength_ = msgHandler.msgLen;
}

//-------------------------------------------------------------------------------------
}
}