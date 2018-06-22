// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Sockets/Public/Sockets.h"

class PacketSender;
class PacketReceiver;
class MemoryStream;
class InterfaceConnect;

/*
	网络模块
	处理连接、收发数据
*/
class KBENGINEPLUGINS_API NetworkInterface
{
public:
	NetworkInterface();
	virtual ~NetworkInterface();

public:
	FSocket* socket() {
		return socket_;
	}

	void process();

	void reset();
	void close();
	bool valid();

	bool connectTo(const FString& addr, uint16 port, InterfaceConnect* callback, int userdata);
	bool send(MemoryStream* pMemoryStream);

	void destroy() {
		isDestroyed_ = true;
	}

private:
	void tickConnecting();

protected:
	FSocket* socket_;
	PacketSender* pPacketSender_;
	PacketReceiver* pPacketReceiver_;

	InterfaceConnect* connectCB_;
	FString connectIP_;
	uint16 connectPort_;
	int connectUserdata_;
	double startTime_;

	bool isDestroyed_;
};
