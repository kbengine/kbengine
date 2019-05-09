// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "NetworkInterfaceBase.h"
#include "ikcp.h"

/*
	网络模块
	处理连接、收发数据
*/
namespace KBEngine
{

class KBENGINEPLUGINS_API NetworkInterfaceKCP : public NetworkInterfaceBase
{
public:
	NetworkInterfaceKCP();
	virtual ~NetworkInterfaceKCP();

	void reset() override;
	void close() override;
	bool valid() override;
	void process() override;

public:

	FSocket* createSocket(const FString& socketDescript = TEXT("default")) override;
	int sendto_(const char *buf, int len);

	ikcpcb*	pKCP() {
		return pKCP_;
	}

	void nextTickKcpUpdate() {
		nextTickKcpUpdate_ = 0;
	}

protected:
	void tickConnecting() override;
	bool _connect(const FInternetAddr& addr) override;

	bool initKCP();
	bool finiKCP();

	static int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user);

	PacketSenderBase* createPacketSender() override;
	PacketReceiverBase* createPacketReceiver() override;

protected:
	ikcpcb*	pKCP_;
	uint32 connID_;
	uint32 nextTickKcpUpdate_;

	TSharedRef<FInternetAddr> addr_;
};

}