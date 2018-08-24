
#include "PacketSender.h"
#include "MemoryStream.h"
#include "KBDebug.h"
#include "NetworkInterfaceBase.h"

PacketSender::PacketSender(NetworkInterfaceBase* pNetworkInterface) :
	pNetworkInterface_(pNetworkInterface)
{
}

PacketSender::~PacketSender()
{
}

bool PacketSender::send(MemoryStream* pMemoryStream)
{
	return true;
}