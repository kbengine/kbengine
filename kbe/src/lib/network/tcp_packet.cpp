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
	data_resize(PACKET_MAX_SIZE_TCP);
	wpos(0);
}

//-------------------------------------------------------------------------------------
TCPPacket::~TCPPacket(void)
{
}

//-------------------------------------------------------------------------------------
int TCPPacket::recvFromEndPoint(EndPoint & ep)
{
	//KBE_ASSERT(MessageHandlers::pMainMessageHandlers != NULL && "Must set up a MainMessageHandlers!\n");

	int len = 0;
	size_t newchunksize = (PACKET_MAX_SIZE_TCP * 2);
	int i = size() / newchunksize;
	
	while(true)
	{
		len = ep.recv(data() + wpos(), PACKET_MAX_SIZE_TCP);
		if(len >= 0)
			wpos(wpos() + len);

		if(len <= 0 || len != PACKET_MAX_SIZE_TCP)
			break;
		
		size_t resize = (++i + 1) * newchunksize;
		if(size() < resize)
			data_resize(resize);
	};
	
	return len;
}

//-------------------------------------------------------------------------------------
}
}