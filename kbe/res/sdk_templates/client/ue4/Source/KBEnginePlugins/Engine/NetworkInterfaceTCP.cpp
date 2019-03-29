
#include "NetworkInterfaceTCP.h"
#include "PacketSenderKCP.h"
#include "MemoryStream.h"
#include "KBEvent.h"
#include "KBDebug.h"
#include "Interfaces.h"
#include "PacketSenderTCP.h"
#include "PacketReceiverTCP.h"

namespace KBEngine
{

NetworkInterfaceTCP::NetworkInterfaceTCP():
	NetworkInterfaceBase()
{
}

NetworkInterfaceTCP::~NetworkInterfaceTCP()
{
}

PacketSenderBase* NetworkInterfaceTCP::createPacketSender()
{
	return new PacketSenderTCP(this);
}

PacketReceiverBase* NetworkInterfaceTCP::createPacketReceiver()
{
	return new PacketReceiverTCP(this);
}

}
