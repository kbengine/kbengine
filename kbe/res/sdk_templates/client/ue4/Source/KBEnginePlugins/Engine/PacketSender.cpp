
#include "PacketSender.h"
#include "MemoryStream.h"
#include "KBDebug.h"

namespace KBEngine
{

PacketSender::PacketSender(NetworkInterface* pNetworkInterface) :
	pNetworkInterface_(pNetworkInterface)
{
}

PacketSender::~PacketSender()
{
}

bool PacketSender::send(MemoryStream* pMemoryStream)
{
	int32 sent = 0;
	return pNetworkInterface_->socket()->Send(pMemoryStream->data() + pMemoryStream->rpos(), pMemoryStream->length(), sent);
}

}
