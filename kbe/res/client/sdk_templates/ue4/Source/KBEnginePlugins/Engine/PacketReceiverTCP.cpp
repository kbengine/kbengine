
#include "PacketReceiverTCP.h"
#include "KBEngine.h"
#include "NetworkInterfaceBase.h"
#include "MessageReader.h"
#include "KBDebug.h"
#include "MemoryStream.h"

PacketReceiverTCP::PacketReceiverTCP(NetworkInterfaceBase* pNetworkInterface):
	PacketReceiver(pNetworkInterface)
{
}

PacketReceiverTCP::~PacketReceiverTCP()
{
}

void PacketReceiverTCP::process()
{
	FSocket *socket = pNetworkInterface_->socket();
	uint32 DataSize = 0;

	while (socket->HasPendingData(DataSize))
	{
		pBuffer_->resize(FMath::Min(DataSize, 65507u));

		int32 BytesRead = 0;
		if (socket->Recv(pBuffer_->data(), pBuffer_->size(), BytesRead))
		{
			pMessageReader_->process(pBuffer_->data(), 0, BytesRead);
		}
	}
}