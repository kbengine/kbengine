
#include "NetworkInterface.h"
#include "PacketReceiver.h"
#include "PacketSender.h"
#include "MemoryStream.h"
#include "KBEvent.h"
#include "KBDebug.h"

NetworkInterface::NetworkInterface():
	socket_(NULL),
	pPacketSender_(NULL),
	pPacketReceiver_(NULL),
	connectCB_(NULL),
	connectIP_(TEXT("")),
	connectPort_(0),
	connectUserdata_(0),
	startTime_(0.0),
	isDestroyed_(false)
{
}

NetworkInterface::~NetworkInterface()
{
	close();
}

void NetworkInterface::reset()
{
	close();
}

void NetworkInterface::close()
{
	if (socket_)
	{
		socket_->Close();
		INFO_MSG("NetworkInterface::close(): network closed!");
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(socket_);
		KBENGINE_EVENT_FIRE("onDisconnected", NewObject<UKBEventData_onDisconnected>());
	}

	socket_ = NULL;

	KBE_SAFE_RELEASE(pPacketSender_);
	KBE_SAFE_RELEASE(pPacketReceiver_);

	connectCB_ = NULL;
	connectIP_ = TEXT("");
	connectPort_ = 0;
	connectUserdata_ = 0;
	startTime_ = 0.0;
}

bool NetworkInterface::valid()
{
	return socket_ != NULL;
}

bool NetworkInterface::connectTo(const FString& addr, uint16 port, InterfaceConnect* callback, int userdata)
{
	INFO_MSG("NetworkInterface::connectTo(): will connect to %s:%d ...", *addr, port);

	reset();

	FIPv4Address ip;
	uint32 OutIP = 0;

	if (!FIPv4Address::Parse(addr, ip))
	{
		auto resolveInfo = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostByName(TCHAR_TO_ANSI(*addr));
		while (!resolveInfo->IsComplete());

		if (resolveInfo->GetErrorCode() != 0)
		{
			ERROR_MSG("NetworkInterface::connectTo(): GetHostByName(%s) error, code=%d", *addr, resolveInfo->GetErrorCode());
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

	socket_ = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	if (!valid())
	{
		ERROR_MSG("NetworkInterface::connectTo(): socket could't be created!");
		return false;
	}
	
	if (!socket_->SetNonBlocking(true))
	{
		ERROR_MSG("NetworkInterface::connectTo(): socket->SetNonBlocking error(%d)!", *addr, port,
			(int32)ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode());
	}

	socket_->Connect(*internetAddr);

	connectCB_ = callback;
	connectIP_ = addr;
	connectPort_ = port;
	connectUserdata_ = userdata;
	startTime_ = getTimeSeconds();

	return true;
}

bool NetworkInterface::send(MemoryStream* pMemoryStream)
{
	if (!valid())
	{
		return false;
	}

	if (!pPacketSender_)
		pPacketSender_ = new PacketSender(this);

	return pPacketSender_->send(pMemoryStream);
}

void NetworkInterface::process()
{
	if (!valid())
		return;

	if (connectCB_)
	{
		tickConnecting();
		return;
	}

	if (!pPacketReceiver_)
		pPacketReceiver_ = new PacketReceiver(this);

	pPacketReceiver_->process();

	if (isDestroyed_)
	{
		delete this;
		return;
	}
}

void NetworkInterface::tickConnecting()
{
	ESocketConnectionState state = socket_->GetConnectionState();

	if (state == SCS_Connected)
	{
		TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		socket_->GetPeerAddress(*addr);

		INFO_MSG("NetworkInterface::tickConnecting(): connect to %s success!", *addr->ToString(true));
		connectCB_->onConnectCallback(connectIP_, connectPort_, true, connectUserdata_);
		connectCB_ = NULL;

		UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
		pEventData->success = true;
		pEventData->address = FString::Printf(TEXT("%s:%d"), *connectIP_, connectPort_);
		KBENGINE_EVENT_FIRE("onConnectionState", pEventData);
	}
	else
	{
		// 如果连接超时则回调失败
		double currTime = getTimeSeconds();
		if (state == SCS_ConnectionError || currTime - startTime_ > 30)
		{
			ERROR_MSG("NetworkInterface::tickConnecting(): connect to %s:%d timeout!", *connectIP_, connectPort_);
			connectCB_->onConnectCallback(connectIP_, connectPort_, false, connectUserdata_);
			connectCB_ = NULL;

			UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
			pEventData->success = false;
			pEventData->address = FString::Printf(TEXT("%s:%d"), *connectIP_, connectPort_);
			KBENGINE_EVENT_FIRE("onConnectionState", pEventData);
		}
	}
}
