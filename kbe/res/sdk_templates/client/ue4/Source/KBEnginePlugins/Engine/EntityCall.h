// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

/*
	实体的EntityCall
	关于EntityCall请参考API手册中对它的描述
	https://github.com/kbengine/kbengine/tree/master/docs/api
*/
namespace KBEngine
{

class Bundle;

class KBENGINEPLUGINS_API EntityCall
{
public:
	EntityCall(int32 eid, const FString& ename);
	virtual ~EntityCall();

	enum ENTITYCALL_TYPE
	{
		ENTITYCALL_TYPE_CELL = 0,		// CELL_ENTITYCALL
		ENTITYCALL_TYPE_BASE = 1		// BASE_ENTITYCALL
	};

public:
	bool isBase()
	{
		return type == ENTITYCALL_TYPE_BASE;
	}

	bool isCell()
	{
		return type == ENTITYCALL_TYPE_CELL;
	}

	/*
		创建新的call
	*/
	Bundle* newCall();
	Bundle* newCall(const FString& methodName, uint16 entitycomponentPropertyID = 0);

	/*
		向服务端发送这个call
	*/
	void sendCall(Bundle* inBundle);

public:
	ENTITY_ID id;
	FString className;
	ENTITYCALL_TYPE type;
	Bundle* pBundle;
};

}