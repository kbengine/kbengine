
#include "PacketSenderKCP.h"
#include "MemoryStream.h"
#include "KBDebug.h"
#include "NetworkInterfaceKCP.h"

namespace KBEngine
{

PacketSenderKCP::PacketSenderKCP(NetworkInterfaceBase* pNetworkInterface) :
	PacketSenderBase(pNetworkInterface)
{
}

PacketSenderKCP::~PacketSenderKCP()
{
}

bool PacketSenderKCP::send(MemoryStream* pMemoryStream)
{
	NetworkInterfaceKCP* pNetworkInterfaceKCP = ((NetworkInterfaceKCP*)pNetworkInterface_);

	pNetworkInterfaceKCP->nextTickKcpUpdate();

	if (ikcp_send(pNetworkInterfaceKCP->pKCP(), (const char*)pMemoryStream->data(), pMemoryStream->length()) < 0)
	{
		ERROR_MSG("PacketSenderKCP::send: send error! currPacketSize%d, ikcp_waitsnd=%d\n",
			pMemoryStream->length(), ikcp_waitsnd(pNetworkInterfaceKCP->pKCP()));

		return false;
	}

	return true;
}

}