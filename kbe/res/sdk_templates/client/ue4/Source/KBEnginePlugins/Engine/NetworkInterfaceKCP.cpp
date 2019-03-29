
#include "NetworkInterfaceKCP.h"
#include "PacketReceiverBase.h"
#include "PacketSenderBase.h"
#include "MemoryStream.h"
#include "KBEvent.h"
#include "KBDebug.h"
#include "Interfaces.h"
#include "KBEngine.h"
#include "KBEngineArgs.h"
#include "PacketSenderKCP.h"
#include "PacketReceiverKCP.h"

namespace KBEngine
{

NetworkInterfaceKCP::NetworkInterfaceKCP():
	NetworkInterfaceBase(),
	pKCP_(NULL),
	connID_(0),
	nextTickKcpUpdate_(0),
	addr_(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr())
{
}

NetworkInterfaceKCP::~NetworkInterfaceKCP()
{
}

void NetworkInterfaceKCP::reset()
{
	NetworkInterfaceBase::reset();
}

void NetworkInterfaceKCP::close()
{
	finiKCP();
	NetworkInterfaceBase::close();
}

bool NetworkInterfaceKCP::valid()
{
	return socket_ && (pKCP() || connectCB_);
}

bool NetworkInterfaceKCP::initKCP()
{
	pKCP_ = ikcp_create((IUINT32)connID_, (void*)this);
	pKCP_->output = &NetworkInterfaceKCP::kcp_output;
	ikcp_setmtu(pKCP_, 1400);

	ikcp_wndsize(pKCP_, KBEngineApp::getSingleton().getInitArgs()->getUDPSendBufferSize(), KBEngineApp::getSingleton().getInitArgs()->getUDPRecvBufferSize());
	ikcp_nodelay(pKCP_, 1, 10, 2, 1);
	pKCP_->rx_minrto = 10;
	nextTickKcpUpdate_ = 0;
	return true;
}

bool NetworkInterfaceKCP::finiKCP()
{
	if (!pKCP_)
		return true;

	ikcp_release(pKCP_);
	pKCP_ = NULL;

	return true;
}

int NetworkInterfaceKCP::kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	NetworkInterfaceKCP* pNetworkInterfaceKCP = (NetworkInterfaceKCP*)user;
	return pNetworkInterfaceKCP->sendto_(buf, len);
}

int NetworkInterfaceKCP::sendto_(const char *buf, int len)
{
	int32 sent = 0;
	socket_->SendTo((uint8*)buf, len, sent, *addr_);

	return 0;
}

FSocket* NetworkInterfaceKCP::createSocket(const FString& socketDescript)
{
	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, socketDescript, false);
}

PacketSenderBase* NetworkInterfaceKCP::createPacketSender()
{
	return new PacketSenderKCP(this);
}

PacketReceiverBase* NetworkInterfaceKCP::createPacketReceiver()
{
	return new PacketReceiverKCP(this);
}

bool NetworkInterfaceKCP::_connect(const FInternetAddr& addr)
{
	MemoryStream s;
	s << UDP_HELLO;

	int32 sent = 0;
	socket_->SendTo(s.data(), s.length(), sent, addr);

	uint32 ip;
	addr.GetIp(ip);
	addr_ = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr(ip, addr.GetPort());
	return true;
}

void NetworkInterfaceKCP::tickConnecting()
{
	uint32 DataSize = 0;

	if(socket_->HasPendingData(DataSize))
	{
		MemoryStream s;
		s.resize(FMath::Min(DataSize, 65507u));

		TSharedRef<FInternetAddr> remoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		int32 BytesRead = 0;
		if (socket_->RecvFrom(s.data(), s.size(), BytesRead, *remoteAddr))
		{
			FString helloAck, versionString;
			uint32 connID;

			s >> helloAck >> versionString >> connID;

			bool success = true;

			if (helloAck != UDP_HELLO_ACK)
			{
				ERROR_MSG("NetworkInterfaceKCP::connectTo(): failed to connect to '%s:%d'! receive hello-ack(%s!=%s) mismatch!", *connectIP_, connectPort_,
					*helloAck, *UDP_HELLO_ACK);

				success = false;
			}
			else if (KBEngineApp::getSingleton().serverVersion() != versionString)
			{
				ERROR_MSG("NetworkInterfaceKCP::connectTo(): failed to connect to '%s:%d'! version(%s!=%s) mismatch!",
					*connectIP_, connectPort_, *versionString, *KBEngineApp::getSingleton().serverVersion());

				success = false;
			}
			else if (connID == 0)
			{
				ERROR_MSG("NetworkInterfaceKCP::connectTo(): failed to connect to '%s:%d'! conv is 0!",
					*connectIP_, connectPort_);

				success = false;
			}
			else
			{
				INFO_MSG("NetworkInterfaceKCP::tickConnecting(): connect to %s:%d success!", *connectIP_, connectPort_);
				connID_ = connID;
				initKCP();
			}

			connectCB_->onConnectCallback(connectIP_, connectPort_, success, connectUserdata_);
			connectCB_ = NULL;

			UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
			pEventData->success = success;
			pEventData->address = FString::Printf(TEXT("%s:%d"), *connectIP_, connectPort_);
			KBENGINE_EVENT_FIRE(KBEventTypes::onConnectionState, pEventData);
		}
	}
	else
	{
		double currTime = getTimeSeconds();
		if (currTime - startTime_ > 30)
		{
			ERROR_MSG("NetworkInterfaceKCP::tickConnecting(): connect to %s:%d timeout!", *connectIP_, connectPort_);
			connectCB_->onConnectCallback(connectIP_, connectPort_, false, connectUserdata_);
			connectCB_ = NULL;

			UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
			pEventData->success = false;
			pEventData->address = FString::Printf(TEXT("%s:%d"), *connectIP_, connectPort_);
			KBENGINE_EVENT_FIRE(KBEventTypes::onConnectionState, pEventData);
		}
	}
}

void NetworkInterfaceKCP::process()
{
	if (pKCP_)
	{
		uint64 secs64 = FPlatformTime::Seconds() * 1000;

		uint32 current = secs64 & 0xfffffffful;
		if (current >= nextTickKcpUpdate_)
		{
			ikcp_update(pKCP_, current);
			nextTickKcpUpdate_ = ikcp_check(pKCP_, current);
		}
	}

	NetworkInterfaceBase::process();
}

}