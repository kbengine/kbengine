// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

namespace KBEngine
{

class NetworkInterfaceBase;
class MessageReader;
class MemoryStream;

/*
	包接收模块(与服务端网络部分的名称对应)
	处理网络数据的接收
*/
class KBENGINEPLUGINS_API PacketReceiverBase
{
public:
	PacketReceiverBase(NetworkInterfaceBase* pNetworkInterface);
	virtual ~PacketReceiverBase();

public:
	virtual void process();

protected:
	NetworkInterfaceBase * pNetworkInterface_;
	MessageReader* pMessageReader_;
	MemoryStream* pBuffer_;
};

}