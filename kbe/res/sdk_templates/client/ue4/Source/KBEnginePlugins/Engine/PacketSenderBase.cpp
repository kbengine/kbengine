
#include "PacketSenderBase.h"
#include "MemoryStream.h"
#include "KBDebug.h"
#include "NetworkInterfaceBase.h"

namespace KBEngine
{

PacketSenderBase::PacketSenderBase(NetworkInterfaceBase* pNetworkInterface) :
	pNetworkInterface_(pNetworkInterface)
{
}

PacketSenderBase::~PacketSenderBase()
{
}

bool PacketSenderBase::send(MemoryStream* pMemoryStream)
{
	return true;
}

}