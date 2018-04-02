
#include "PacketReceiver.h"
#include "KBEngine.h"
#include "NetworkInterface.h"
#include "MessageReader.h"
#include "KBDebug.h"

PacketReceiver::PacketReceiver(NetworkInterface* pNetworkInterface):
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