// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

namespace KBEngine
{

/*
接口模块
用于声明和实现某些回调统一接口
*/
class KBENGINEPLUGINS_API InterfaceConnect
{
public:

	virtual void onConnectCallback(FString ip, uint16 port, bool success, int userdata) = 0;

};


}