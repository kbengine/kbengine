
#include "NetworkInterfaceTCP.h"
#include "PacketSenderKCP.h"
#include "PacketSender.h"
#include "MemoryStream.h"
#include "KBEvent.h"
#include "KBDebug.h"
#include "Interfaces.h"
#include "PacketSenderTCP.h"
#include "PacketReceiverTCP.h"

NetworkInterfaceTCP::NetworkInterfaceTCP():
	NetworkInterfaceBase()
{
}

NetworkInterfaceTCP::~NetworkInterfaceTCP()
{
}

PacketSender* NetworkInterfaceTCP::createPacketSender()
{
	return new PacketSenderTCP(this);
}

PacketReceiver* NetworkInterfaceTCP::createPacketReceiver()
{
	return new PacketReceiverTCP(this);
}

