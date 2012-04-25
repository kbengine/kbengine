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
	packets_(),
	isTCPPacket_(pt == TCP_PACKET)
{
	 newPacket();
}

//-------------------------------------------------------------------------------------
Bundle::Bundle(Packet * p, PacketType pt):
	pChannel_(NULL),
	pCurrPacket_(NULL),
	packets_(),
	isTCPPacket_(pt == TCP_PACKET)
{
	pCurrPacket_ = p;
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
	packets_.push_back(pCurrPacket_);
	pCurrPacket_ = NULL;
}

//-------------------------------------------------------------------------------------
void Bundle::clear()
{
	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
		delete (*iter);
	
	packets_.clear();
	SAFE_RELEASE(pCurrPacket_);
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
		ep.send(pPacket->data(), pPacket->size());
		delete pPacket;
	}
	
	packets_.clear();
}

//-------------------------------------------------------------------------------------
void Bundle::newMessage(const MessageHandler& msgHandler)
{
	KBE_ASSERT(pCurrPacket_ != NULL);
	
	(*pCurrPacket_) << msgHandler.msgID;
	numMessages_++;
}

//-------------------------------------------------------------------------------------
}
}