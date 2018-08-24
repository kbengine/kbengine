
#include "PacketReceiver.h"
#include "KBEngine.h"
#include "NetworkInterfaceBase.h"
#include "MessageReader.h"
#include "KBDebug.h"
#include "MemoryStream.h"

PacketReceiver::PacketReceiver(NetworkInterfaceBase* pNetworkInterface):
	pNetworkInterface_(pNetworkInterface),
	pMessageReader_(new MessageReader()),
	pBuffer_(new MemoryStream())
{
}

PacketReceiver::~PacketReceiver()
{
	KBE_SAFE_RELEASE(pMessageReader_);
	KBE_SAFE_RELEASE(pBuffer_);
	
	INFO_MSG("PacketReceiver::~PacketReceiver(), destroyed!");
}

void PacketReceiver::process()
{
}