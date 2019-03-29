// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

namespace KBEngine
{

class MemoryStream;
class NetworkInterfaceBase;
class Message;

/*
	这个模块将多个数据包打捆在一起
	由于每个数据包都有最大上限， 向Bundle中写入大量数据将会在内部产生多个MemoryStream
	在send时会全部发送出去
*/
class KBENGINEPLUGINS_API Bundle
{
public:
	Bundle();
	virtual ~Bundle();

public:
	static Bundle* createObject();
	static void reclaimObject(Bundle* obj);

	void newMessage(Message* pMsg);
	void fini(bool issend);

	void writeMsgLength();
	void send(NetworkInterfaceBase* pNetworkInterface);

	void checkStream(uint32 v);

	void clear();

public:
	Bundle &operator<<(uint8 value);
	Bundle &operator<<(uint16 value);
	Bundle &operator<<(uint32 value);
	Bundle &operator<<(uint64 value);
	Bundle &operator<<(int8 value);
	Bundle &operator<<(int16 value);
	Bundle &operator<<(int32 value);
	Bundle &operator<<(int64 value);
	Bundle &operator<<(float value);
	Bundle &operator<<(double value);
	Bundle &operator<<(bool value);

	Bundle &operator<<(const FString &value);
	Bundle &operator<<(const char *str);

	void appendBlob(const TArray<uint8>& datas);
	void appendUTF8String(const FString& str);

	void writeInt8(int8 v)
	{
		(*this) << v;
	}

	void writeInt16(int16 v)
	{
		(*this) << v;
	}
		
	void writeInt32(int32 v)
	{
		(*this) << v;
	}

	void writeInt64(int64 v)
	{
		(*this) << v;
	}
	
	void writeUint8(uint8 v)
	{
		(*this) << v;
	}

	void writeUint16(uint16 v)
	{
		(*this) << v;
	}
		
	void writeUint32(uint32 v)
	{
		(*this) << v;
	}

	void writeUint64(uint64 v)
	{
		(*this) << v;
	}
	
	void writeFloat(float v)
	{
		(*this) << v;
	}

	void writeDouble(double v)
	{
		(*this) << v;
	}
	
	void writeString(const FString& v)
	{
		(*this) << v;
	}

	void writeUnicode(const FString& v)
	{
		appendUTF8String(v);
	}
	
	void writeBlob(const TArray<uint8>& v)
	{
		appendBlob(v);
	}

	void writePython(const TArray<uint8>& v)
	{
		appendBlob(v);
	}

	void writeEntitycall(const TArray<uint8>& v)
	{
		uint64 cid = 0;
		int32 id = 0;
		uint16 type = 0;
		uint16 utype = 0;

		(*this) << cid << id << type << utype;
	}

	void writeVector2(const FVector2D& v);
	void writeVector3(const FVector& v);
	void writeVector4(const FVector4& v);

protected:
	MemoryStream* pCurrPacket_;
	TArray<MemoryStream*> streams_;

	int numMessage_;
	int messageLength_;
	Message* pMsgtype_;
	int curMsgStreamIndex_;
};

}