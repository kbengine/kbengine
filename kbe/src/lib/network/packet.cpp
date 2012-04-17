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
//-------------------------------------------------------------------------------------
Packet::Packet()
{
}

//-------------------------------------------------------------------------------------
Packet::~Packet(void)
{
}

//-------------------------------------------------------------------------------------
int Packet::recvFromEndPoint(Socket & ep)
{
	int len = ep.recv(data_, PACKET_MAX_SIZE);

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