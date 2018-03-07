// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"
#include "KBVar.h"

class MemoryStream;

/*
	entitydef所支持的基本数据类型
	改模块中的类抽象出了所有的支持类型并提供了这些类型的数据序列化成二进制数据与反序列化操作（主要用于网络通讯的打包与解包）
*/
class KBENGINEPLUGINS_API KBEDATATYPE_BASE
{
public:
		KBEDATATYPE_BASE()
		{
		}
		
		virtual ~KBEDATATYPE_BASE()
		{
		}

		virtual void bind()
		{
		}

		virtual KBVar* createFromStream(MemoryStream& stream)
		{
			return NULL;
		}
		
		virtual void addToStream(Bundle& stream, KBVar& v)
		{
		}
		
		virtual KBVar* parseDefaultValStr(const FString& v)
		{
			return NULL;
		}
		
		virtual bool isSameType(KBVar& v)
		{
			return false;
		}
		
		virtual FString c_str() const {
			return TEXT("UNKNOWN");
		}

protected:

};

class KBENGINEPLUGINS_API KBEDATATYPE_INT8 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("INT8");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_INT16 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("INT16");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_INT32 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("INT32");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_INT64 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("INT64");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_UINT8 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("UINT8");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_UINT16 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("UINT6");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_UINT32 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("UINT32");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_UINT64 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("UINT64");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_FLOAT : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("FLOAT");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_DOUBLE : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("DOUBLE");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_STRING : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("STRING");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_VECTOR2 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("VECTOR2");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_VECTOR3 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("VECTOR3");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_VECTOR4 : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("VECTOR4");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_PYTHON : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("PYTHON");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_UNICODE : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("UNICODE|STRING");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_ENTITYCALL : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("ENTITYCALL");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_BLOB : public KBEDATATYPE_BASE
{
public:
	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("BLOB|TArray<uint8>");
	}
};

class KBENGINEPLUGINS_API KBEDATATYPE_ARRAY : public KBEDATATYPE_BASE
{
public:
	KBEDATATYPE_ARRAY() :
		vtype(NULL),
		tmpset_uitemtype(-1)
	{

	}

	virtual void bind() override;

	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("KB_ARRAY");
	}

public:
	KBEDATATYPE_BASE* vtype;
	int tmpset_uitemtype;
};

class KBENGINEPLUGINS_API KBEDATATYPE_FIXED_DICT : public KBEDATATYPE_BASE
{
public:
	KBEDATATYPE_FIXED_DICT() :
		implementedBy(),
		dicttype(),
		dicttype_map()
	{

	}

	virtual void bind() override;

	virtual KBVar* createFromStream(MemoryStream& stream) override;
	virtual void addToStream(Bundle& stream, KBVar& v) override;

	virtual KBVar* parseDefaultValStr(const FString& v) override;

	virtual bool isSameType(KBVar& v) override;

	virtual FString c_str() const {
		return TEXT("KB_FIXED_DICT");
	}

public:
	FString implementedBy;
	TMap<FString, KBEDATATYPE_BASE*> dicttype;
	TMap<FString, uint16> dicttype_map;
};













