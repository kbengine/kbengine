// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "EncryptionFilter.h"



/*
	网络模块
	处理连接、收发数据
*/
namespace KBEngine
{

class PacketSenderBase;
class PacketReceiverBase;
class MemoryStream;
class InterfaceConnect;
class KBENGINEPLUGINS_API NetworkInterfaceBase
{
public:
    NetworkInterfaceBase();
	virtual ~NetworkInterfaceBase();

	const FString UDP_HELLO = TEXT("62a559f3fa7748bc22f8e0766019d498");
	const FString UDP_HELLO_ACK = TEXT("1432ad7c829170a76dd31982c3501eca");

public:
	FSocket* socket() {
		return socket_;
	}

	virtual EncryptionFilter* filter() {
		return pFilter_;
	}

	virtual void setFilter(EncryptionFilter* filter) {
		pFilter_ = filter;
	}

	virtual void process();

	virtual void reset();
	virtual void close();
	virtual bool valid();

	virtual bool connectTo(const FString& addr, uint16 port, InterfaceConnect* callback, int userdata);
	virtual bool send(MemoryStream* pMemoryStream);

	virtual void destroy() {
		isDestroyed_ = true;
	}

	virtual FSocket* createSocket(const FString& socketDescript = TEXT("default"));

protected:
	virtual void tickConnecting();
	virtual bool _connect(const FInternetAddr& addr);

	virtual PacketSenderBase* createPacketSender() = 0;
	virtual PacketReceiverBase* createPacketReceiver() = 0;

protected:
	FSocket* socket_;
	PacketSenderBase* pPacketSender_;
	PacketReceiverBase* pPacketReceiver_;

	InterfaceConnect* connectCB_;
	FString connectIP_;
	uint16 connectPort_;
	int connectUserdata_;
	double startTime_;

	bool isDestroyed_;

	EncryptionFilter *pFilter_;

};

}