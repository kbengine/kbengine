// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

namespace KBEngine
{

class MemoryStream;

/*
	消息阅读模块
	从数据包流中分析出所有的消息包并将其交给对应的消息处理函数
*/
class KBENGINEPLUGINS_API MessageReader
{
public:
	MessageReader();
	virtual ~MessageReader();

	enum READ_STATE
	{
		// 消息ID
		READ_STATE_MSGID = 0,

		// 消息的长度65535以内
		READ_STATE_MSGLEN = 1,

		// 当上面的消息长度都无法到达要求时使用扩展长度
		// uint32
		READ_STATE_MSGLEN_EX = 2,

		// 消息的内容
		READ_STATE_BODY = 3
	};

public:
	void process(const uint8* datas, MessageLengthEx offset, MessageLengthEx length);

protected:
	MessageID msgid_;
	MessageLength msglen_;
	MessageLengthEx expectSize_;
	READ_STATE state_;
	MemoryStream* pMemoryStream_;

};

}