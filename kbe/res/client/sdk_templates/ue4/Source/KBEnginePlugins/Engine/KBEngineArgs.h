// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"


/*
初始化KBEngine的参数类
*/
class KBENGINEPLUGINS_API KBEngineArgs
{
public:
	KBEngineArgs();
	virtual ~KBEngineArgs();

	int getRecvBufferSize();
	int getSendBufferSize();

public:
	FString ip;
	int port;

	// 客户端类型
	// Reference: http://www.kbengine.org/docs/programming/clientsdkprogramming.html, client types
	EKCLIENT_TYPE clientType;

	bool syncPlayer;
	bool useAliasEntityID;
	bool isOnInitCallPropertysSetMethods;

	// 心跳频率（tick数）
	int serverHeartbeatTick;

	// 发送缓冲大小
	MessageLengthEx SEND_BUFFER_MAX;

	// 接收缓冲区大小
	MessageLengthEx RECV_BUFFER_MAX;
};
