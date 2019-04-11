
#include "PacketReceiverKCP.h"
#include "KBEngine.h"
#include "NetworkInterfaceKCP.h"
#include "MessageReader.h"
#include "KBDebug.h"
#include "MemoryStream.h"

namespace KBEngine
{

PacketReceiverKCP::PacketReceiverKCP(NetworkInterfaceBase* pNetworkInterface):
	PacketReceiverBase(pNetworkInterface),
	remoteAddr_(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr())
{
	pBuffer_->resize(65507u);
}

PacketReceiverKCP::~PacketReceiverKCP()
{
}

void PacketReceiverKCP::process()
{
	FSocket *socket = pNetworkInterface_->socket();
	uint32 DataSize = 0;
	
	while (socket->HasPendingData(DataSize))
	{
		int32 BytesRead = 0;

		if (socket->RecvFrom(pBuffer_->data(), pBuffer_->size(), BytesRead, *remoteAddr_))
		{
			NetworkInterfaceKCP* pNetworkInterfaceKCP = ((NetworkInterfaceKCP*)pNetworkInterface_);
			
			pNetworkInterfaceKCP->nextTickKcpUpdate();
			pBuffer_->wpos(BytesRead);

			if (ikcp_input(pNetworkInterfaceKCP->pKCP(), (const char*)pBuffer_->data(), pBuffer_->length()) < 0)
			{
				continue; 
			}

			while (true)
			{
				BytesRead = ikcp_recv(pNetworkInterfaceKCP->pKCP(), (char*)pBuffer_->data(), pBuffer_->size());
				if (BytesRead < 0)
				{
					//WARNING_MSG("PacketReceiverKCP::process(): recvd_bytes(%d) <= 0! addr=%s\n", BytesRead, *remoteAddr_);
					break;
				}
				else
				{
					if (BytesRead >= (int)pBuffer_->size())
					{
						ERROR_MSG("PacketReceiverKCP::process(): recvd_bytes(%d) >= maxBuf(%d)! addr=%s\n", BytesRead, pBuffer_->size(), *(*remoteAddr_).ToString(true));
					}

					pBuffer_->wpos(BytesRead);

					if (pNetworkInterface_->filter())
					{
						pNetworkInterface_->filter()->recv(pMessageReader_, pBuffer_);
					}
					else
					{
						pMessageReader_->process(pBuffer_->data(), 0, BytesRead);
					}
				}
			}
		}
	}
}

}