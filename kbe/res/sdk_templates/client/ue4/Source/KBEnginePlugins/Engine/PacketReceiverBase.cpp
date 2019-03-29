
#include "PacketReceiverBase.h"
#include "KBEngine.h"
#include "NetworkInterfaceBase.h"
#include "MessageReader.h"
#include "KBDebug.h"
#include "MemoryStream.h"

namespace KBEngine
{

PacketReceiverBase::PacketReceiverBase(NetworkInterfaceBase* pNetworkInterface):
	pNetworkInterface_(pNetworkInterface),
	pMessageReader_(new MessageReader()),
	pBuffer_(new MemoryStream())
{
}

PacketReceiverBase::~PacketReceiverBase()
{
	KBE_SAFE_RELEASE(pMessageReader_);
	KBE_SAFE_RELEASE(pBuffer_);
	
	INFO_MSG("PacketReceiverBase::~PacketReceiverBase(), destroyed!");
}

void PacketReceiverBase::process()
{
}

}
