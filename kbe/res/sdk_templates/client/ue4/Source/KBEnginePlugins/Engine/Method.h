// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "KBVar.h"
#include "KBECommon.h"

namespace KBEngine
{

class DATATYPE_BASE;

/*
	实体定义的方法模块
	抽象出一个def文件中定义的方法，改模块类提供了该方法的相关描述信息
	例如：方法的参数、方法的id、方法对应脚本的handler
*/
class KBENGINEPLUGINS_API Method
{
public:
	Method();
	virtual ~Method();

public:
	FString name;
	uint16 methodUtype;
	int16 aliasID;

	TArray<DATATYPE_BASE*> args;
};

}