#include "tcp_packet.hpp"
#ifndef CODE_INLINE
#include "tcp_packet.ipp"
#endif
#include "network/bundle.hpp"
#include "network/endpoint.hpp"
#include "network/network_interface.hpp"
#include "network/message_handler.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
TCPPacket::TCPPacket(MessageID msgID, size_t res):
Packet(msgID, res)
{
	resize(PACKET_MAX_SIZE_TCP);
}

//-------------------------------------------------------------------------------------
TCPPacket::~TCPPacket(void)
{
}

//-------------------------------------------------------------------------------------
int TCPPacket::recvFromEndPoint(EndPoint & ep)
{
	//KBE_ASSERT(MessageHandlers::pMainMessageHandlers != NULL && "Must set up a MainMessageHandlers!\n");

	int len = ep.recv(data(), PACKET_MAX_SIZE_TCP);
	wpos(len);
	return len;
}

//-------------------------------------------------------------------------------------
}
}