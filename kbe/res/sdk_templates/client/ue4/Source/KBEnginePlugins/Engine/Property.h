// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "KBVar.h"
#include "KBECommon.h"


namespace KBEngine
{

class DATATYPE_BASE;

/*
	抽象出一个entitydef中定义的属性
	该模块描述了属性的id以及数据类型等信息
*/
class KBENGINEPLUGINS_API Property
{
public:
	Property();
	virtual ~Property();

	bool isBase()
	{
		return properFlags == (uint32)ED_FLAG_BASE_AND_CLIENT ||
			properFlags == (uint32)ED_FLAG_BASE;
	}

	bool isOwnerOnly()
	{
		return properFlags == (uint32)ED_FLAG_CELL_PUBLIC_AND_OWN ||
			properFlags == (uint32)ED_FLAG_OWN_CLIENT;
	}

	bool isOtherOnly()
	{
		return properFlags == (uint32)ED_FLAG_OTHER_CLIENTS ||
			properFlags == (uint32)ED_FLAG_OTHER_CLIENTS;
	}

public:
	FString name;
	DATATYPE_BASE* pUtype;
	uint16 properUtype;
	uint32 properFlags;
	int16 aliasID;

	FString defaultValStr;
	KBVar* pDefaultVal;

};

}