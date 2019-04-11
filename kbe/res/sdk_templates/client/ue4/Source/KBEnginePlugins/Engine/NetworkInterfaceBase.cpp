
#include "NetworkInterfaceBase.h"
#include "PacketReceiverBase.h"
#include "PacketSenderBase.h"
#include "MemoryStream.h"
#include "KBEvent.h"
#include "KBDebug.h"
#include "Interfaces.h"
#include "KBEngine.h"

namespace KBEngine
{

NetworkInterfaceBase::NetworkInterfaceBase():
	socket_(NULL),
	pPacketSender_(NULL),
	pPacketReceiver_(NULL),
	connectCB_(NULL),
	connectIP_(TEXT("")),
	connectPort_(0),
	connectUserdata_(0),
	startTime_(0.0),
	isDestroyed_(false),
	pFilter_(NULL)
{
}

NetworkInterfaceBase::~NetworkInterfaceBase()
{
	close();
}

void NetworkInterfaceBase::reset()
{
	close();
}

void NetworkInterfaceBase::close()
{
	if (socket_)
	{
		socket_->Close();
		INFO_MSG("NetworkInterfaceBase::close(): network closed!");
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(socket_);
		KBENGINE_EVENT_FIRE(KBEventTypes::onDisconnected, NewObject<UKBEventData_onDisconnected>());
	}

	socket_ = NULL;

	KBE_SAFE_RELEASE(pPacketSender_);
	KBE_SAFE_RELEASE(pPacketReceiver_);
	KBE_SAFE_RELEASE(pFilter_);

	connectCB_ = NULL;
	connectIP_ = TEXT("");
	connectPort_ = 0;
	connectUserdata_ = 0;
	startTime_ = 0.0;
}

bool NetworkInterfaceBase::valid()
{
	return socket_ != NULL;
}

FSocket* NetworkInterfaceBase::createSocket(const FString& socketDescript)
{
	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, socketDescript, false);
}

bool NetworkInterfaceBase::_connect(const FInternetAddr& addr)
{
	return socket_->Connect(addr);
}

bool NetworkInterfaceBase::connectTo(const FString& addr, uint16 port, InterfaceConnect* callback, int userdata)
{
	INFO_MSG("NetworkInterfaceBase::connectTo(): will connect to %s:%d ...", *addr, port);

	reset();

	FIPv4Address ip;
	uint32 OutIP = 0;

	if (!FIPv4Address::Parse(addr, ip))
	{
		auto resolveInfo = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostByName(TCHAR_TO_ANSI(*addr));
		while (!resolveInfo->IsComplete());

		if (resolveInfo->GetErrorCode() != 0)
		{
			ERROR_MSG("NetworkInterfaceBase::connectTo(): GetHostByName(%s) error, code=%d", *addr, resolveInfo->GetErrorCode());
			return false;
		}

		resolveInfo->GetResolvedAddress().GetIp(OutIP);
	}
	else
	{
		OutIP = ip.Value;
	}

	TSharedRef<FInternetAddr> internetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	internetAddr->SetIp(OutIP);
	internetAddr->SetPort(port);

	socket_ = createSocket();

	if (!socket_)
	{
		ERROR_MSG("NetworkInterfaceBase::connectTo(): socket could't be created!");
		return false;
	}
	
	if (!socket_->SetNonBlocking(true))
	{
		ERROR_MSG("NetworkInterfaceBase::connectTo(%s:%d): socket->SetNonBlocking error(%d)!", *addr, port,
			(int32)ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode());
	}

	if (!_connect(*internetAddr))
		return false;

	connectCB_ = callback;
	connectIP_ = addr;
	connectPort_ = port;
	connectUserdata_ = userdata;
	startTime_ = getTimeSeconds();

	return true;
}

bool NetworkInterfaceBase::send(MemoryStream* pMemoryStream)
{
	if (!valid())
	{
		return false;
	}

	if (!pPacketSender_)
		pPacketSender_ = createPacketSender();

	if (pFilter_ )
		return pFilter_->send(pPacketSender_, pMemoryStream);

	return pPacketSender_->send(pMemoryStream);
}

void NetworkInterfaceBase::process()
{
	if (!valid())
		return;

	if (connectCB_)
	{
		tickConnecting();
		return;
	}

	if (!pPacketReceiver_)
		pPacketReceiver_ = createPacketReceiver();

	pPacketReceiver_->process();

	if (isDestroyed_)
	{
		delete this;
		return;
	}
}

void NetworkInterfaceBase::tickConnecting()
{
	ESocketConnectionState state = socket_->GetConnectionState();

	if (state == SCS_Connected)
	{
		TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		socket_->GetPeerAddress(*addr);

		INFO_MSG("NetworkInterfaceBase::tickConnecting(): connect to %s success!", *addr->ToString(true));
		connectCB_->onConnectCallback(connectIP_, connectPort_, true, connectUserdata_);
		connectCB_ = NULL;

		UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
		pEventData->success = true;
		pEventData->address = FString::Printf(TEXT("%s:%d"), *connectIP_, connectPort_);
		KBENGINE_EVENT_FIRE(KBEventTypes::onConnectionState, pEventData);
	}
	else
	{
		// 如果连接超时则回调失败
		double currTime = getTimeSeconds();
		if (state == SCS_ConnectionError || currTime - startTime_ > 30)
		{
			ERROR_MSG("NetworkInterfaceBase::tickConnecting(): connect to %s:%d timeout!", *connectIP_, connectPort_);
			connectCB_->onConnectCallback(connectIP_, connectPort_, false, connectUserdata_);
			connectCB_ = NULL;

			UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
			pEventData->success = false;
			pEventData->address = FString::Printf(TEXT("%s:%d"), *connectIP_, connectPort_);
			KBENGINE_EVENT_FIRE(KBEventTypes::onConnectionState, pEventData);
		}
	}
}

}