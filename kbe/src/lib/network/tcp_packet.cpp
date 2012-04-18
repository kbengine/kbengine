#include "tcp_packet.hpp"
#ifndef CODE_INLINE
#include "tcp_packet.ipp"
#endif
#include "network/bundle.hpp"
#include "network/endpoint.hpp"
#include "network/network_interface.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
TCPPacket::TCPPacket()
{
}

//-------------------------------------------------------------------------------------
TCPPacket::~TCPPacket(void)
{
}

//-------------------------------------------------------------------------------------
int TCPPacket::recvFromEndPoint(EndPoint & ep)
{
	int len = ep.recv(data_, TCP_PACKET_MAX_SIZE);

	if (len >= 0)
	{
		this->msgEndOffset(len);
	}
	printf("---------------------------[%s]%d\n",data_, len);
	return len;
}

//-------------------------------------------------------------------------------------
}
}