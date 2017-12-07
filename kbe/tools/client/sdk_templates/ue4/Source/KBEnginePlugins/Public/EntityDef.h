// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

class ScriptModule;
class KBEDATATYPE_BASE;

/*
	EntityDef模块
	管理了所有的实体定义的描述以及所有的数据类型描述
*/
class KBENGINEPLUGINS_API EntityDef
{
public:
	

	// 所有的数据类型
	static TMap<FString, uint16> datatype2id;
	static TMap<FString, KBEDATATYPE_BASE*> datatypes;
	static TMap<uint16, KBEDATATYPE_BASE*> id2datatypes;

	static TMap<FString, int32> entityclass;

	static TMap<FString, ScriptModule*> moduledefs;
	static TMap<uint16, ScriptModule*> idmoduledefs;

	static void initialize();

	static void clear();
	static void initDataType();
	static void bindMessageDataType();
};
