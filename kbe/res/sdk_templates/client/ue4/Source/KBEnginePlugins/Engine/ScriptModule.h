// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "KBECommon.h"


namespace KBEngine
{

class Property;
class Method;
class Entity;

/*
	一个entitydef中定义的脚本模块的描述类
	包含了某个entity定义的属性与方法以及该entity脚本模块的名称与模块ID
*/
class KBENGINEPLUGINS_API ScriptModule
{
public:
	ScriptModule(const FString& moduleName, int type);
	virtual ~ScriptModule();

	Entity* createEntity();

public:
	FString name;
	bool usePropertyDescrAlias;
	bool useMethodDescrAlias;

	TMap<FString, Property*> propertys;
	TMap<uint16, Property*> idpropertys;

	TMap<FString, Method*> methods;
	TMap<FString, Method*> base_methods;
	TMap<FString, Method*> cell_methods;

	TMap<uint16, Method*> idmethods;
	TMap<uint16, Method*> idbase_methods;
	TMap<uint16, Method*> idcell_methods;

	uint16 utype;
};

}
