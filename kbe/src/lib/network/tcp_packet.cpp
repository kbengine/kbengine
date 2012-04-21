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
TCPPacket::TCPPacket(PacketHeaders ph, size_t res):
Packet(ph, res)
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
	int len = ep.recv(data(), PACKET_MAX_SIZE_TCP);
	wpos(len);
	std::string s;
	int io = 0;
	if(len > 0)
	{
		hexlike();
		(*this) >> s;
		(*this) >> io;
	}

	printf("---------------------------[%s-%d]%d--%d-wpo1s=%d\n",s.c_str(), io, len, data_.size(), wpos());
	return len;
}

//-------------------------------------------------------------------------------------
}
}