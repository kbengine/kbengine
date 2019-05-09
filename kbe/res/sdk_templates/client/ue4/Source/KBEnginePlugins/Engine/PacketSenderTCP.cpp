
#include "PacketSenderTCP.h"
#include "MemoryStream.h"
#include "KBDebug.h"
#include "NetworkInterfaceBase.h"

namespace KBEngine
{

PacketSenderTCP::PacketSenderTCP(NetworkInterfaceBase* pNetworkInterface) :
	PacketSenderBase(pNetworkInterface)
{
}

PacketSenderTCP::~PacketSenderTCP()
{
}

bool PacketSenderTCP::send(MemoryStream* pMemoryStream)
{
	int32 sent = 0;
	return pNetworkInterface_->socket()->Send(pMemoryStream->data() + pMemoryStream->rpos(), pMemoryStream->length(), sent);
}

}