// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "NetworkInterfaceBase.h"

/*
	网络模块
	处理连接、收发数据
*/
namespace KBEngine
{

class KBENGINEPLUGINS_API NetworkInterfaceTCP : public NetworkInterfaceBase
{
public:
	NetworkInterfaceTCP();
	virtual ~NetworkInterfaceTCP();
	
protected:
	PacketSenderBase* createPacketSender() override;
	PacketReceiverBase* createPacketReceiver() override;

};

}
