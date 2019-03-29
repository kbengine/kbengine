// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"
#include "PacketSenderBase.h"

namespace KBEngine
{

class NetworkInterfaceBase;
class MemoryStream;

/*
	包发送模块(与服务端网络部分的名称对应)
	处理网络数据的发送
*/
class KBENGINEPLUGINS_API PacketSenderTCP : public PacketSenderBase
{
public:
	PacketSenderTCP(NetworkInterfaceBase* pNetworkInterface);
	virtual ~PacketSenderTCP();

public:
	virtual bool send(MemoryStream* pMemoryStream);

protected:

};

}