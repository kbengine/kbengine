#include "packet.hpp"
#ifndef CODE_INLINE
#include "packet.ipp"
#endif
#include "network/bundle.hpp"
#include "network/socket.hpp"
#include "network/network_interface.hpp"

namespace KBEngine { 
namespace Mercury
{
Packet::Packet()
{
}

Packet::~Packet(void)
{
}

int Packet::recvFromEndPoint(Socket & ep, Address & addr)
{
	int len = ep.recv(data_, PACKET_MAX_SIZE);
	/*
	int len = ep.recvfrom(data_, PACKET_MAX_SIZE, (uint16*)&addr.port, (uint32*)&addr.ip);
*/
	if (len >= 0)
	{
		this->msgEndOffset(len);
	}

	return len;
}
}
}