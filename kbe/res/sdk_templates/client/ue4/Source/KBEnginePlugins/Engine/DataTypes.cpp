
#include "DataTypes.h"
#include "MemoryStream.h"
#include "EntityDef.h"
#include "KBDebug.h"
#include "Runtime/Core/Public/Misc/Variant.h"
#include "Bundle.h"

namespace KBEngine
{

KBVar* DATATYPE_INT8::createFromStream(MemoryStream& stream)
{
	int8 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_INT8::addToStream(Bundle& stream, KBVar& v)
{
	int8 val = v;
	stream << val;
}

KBVar* DATATYPE_INT8::parseDefaultValStr(const FString& v)
{
	int8 val = (int8)FCString::Atoi(*v);
	return new KBVar(val);
}

bool DATATYPE_INT8::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Int8;
}

KBVar* DATATYPE_INT16::createFromStream(MemoryStream& stream)
{
	int16 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_INT16::addToStream(Bundle& stream, KBVar& v)
{
	int16 val = v;
	stream << val;
}

KBVar* DATATYPE_INT16::parseDefaultValStr(const FString& v)
{
	int16 val = (int16)FCString::Atoi(*v);
	return new KBVar(val);
}

bool DATATYPE_INT16::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Int16;
}

KBVar* DATATYPE_INT32::createFromStream(MemoryStream& stream)
{
	int32 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_INT32::addToStream(Bundle& stream, KBVar& v)
{
	int32 val = v;
	stream << val;
}

KBVar* DATATYPE_INT32::parseDefaultValStr(const FString& v)
{
	int32 val = (int32)FCString::Atoi(*v);
	return new KBVar(val);
}

bool DATATYPE_INT32::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Int32;
}

KBVar* DATATYPE_INT64::createFromStream(MemoryStream& stream)
{
	int64 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_INT64::addToStream(Bundle& stream, KBVar& v)
{
	int64 val = v;
	stream << val;
}

KBVar* DATATYPE_INT64::parseDefaultValStr(const FString& v)
{
	int64 val = (int64)FCString::Atoi64(*v);
	return new KBVar(val);
}

bool DATATYPE_INT64::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Int64;
}

KBVar* DATATYPE_UINT8::createFromStream(MemoryStream& stream)
{
	uint8 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_UINT8::addToStream(Bundle& stream, KBVar& v)
{
	uint8 val = v;
	stream << val;
}

KBVar* DATATYPE_UINT8::parseDefaultValStr(const FString& v)
{
	uint8 val = (uint8)FCString::Atoi(*v);
	return new KBVar(val);
}

bool DATATYPE_UINT8::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::UInt8;
}

KBVar* DATATYPE_UINT16::createFromStream(MemoryStream& stream)
{
	uint16 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_UINT16::addToStream(Bundle& stream, KBVar& v)
{
	uint16 val = v;
	stream << val;
}

KBVar* DATATYPE_UINT16::parseDefaultValStr(const FString& v)
{
	uint16 val = (uint16)FCString::Atoi(*v);
	return new KBVar(val);
}

bool DATATYPE_UINT16::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::UInt16;
}

KBVar* DATATYPE_UINT32::createFromStream(MemoryStream& stream)
{
	uint32 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_UINT32::addToStream(Bundle& stream, KBVar& v)
{
	uint32 val = v;
	stream << val;
}

KBVar* DATATYPE_UINT32::parseDefaultValStr(const FString& v)
{
	uint32 val = (uint32)FCString::Atoi(*v);
	return new KBVar(val);
}

bool DATATYPE_UINT32::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::UInt32;
}

KBVar* DATATYPE_UINT64::createFromStream(MemoryStream& stream)
{
	uint64 val = 0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_UINT64::addToStream(Bundle& stream, KBVar& v)
{
	uint64 val = v;
	stream << val;
}

KBVar* DATATYPE_UINT64::parseDefaultValStr(const FString& v)
{
	uint64 val = (uint64)FCString::Atoi64(*v);
	return new KBVar(val);
}

bool DATATYPE_UINT64::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::UInt64;
}

KBVar* DATATYPE_FLOAT::createFromStream(MemoryStream& stream)
{
	float val = 0.f;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_FLOAT::addToStream(Bundle& stream, KBVar& v)
{
	float val = v;
	stream << val;
}

KBVar* DATATYPE_FLOAT::parseDefaultValStr(const FString& v)
{
	float val = (float)FCString::Atof(*v);
	return new KBVar(val);
}

bool DATATYPE_FLOAT::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Float;
}

KBVar* DATATYPE_DOUBLE::createFromStream(MemoryStream& stream)
{
	double val = 0.0;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_DOUBLE::addToStream(Bundle& stream, KBVar& v)
{
	double val = v;
	stream << val;
}

KBVar* DATATYPE_DOUBLE::parseDefaultValStr(const FString& v)
{
	double val = (double)FCString::Atof(*v);
	return new KBVar(val);
}

bool DATATYPE_DOUBLE::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Double;
}

KBVar* DATATYPE_STRING::createFromStream(MemoryStream& stream)
{
	FString val;
	stream >> val;
	return new KBVar(val);
}

void DATATYPE_STRING::addToStream(Bundle& stream, KBVar& v)
{
	FString val = v;
	stream << val;
}

KBVar* DATATYPE_STRING::parseDefaultValStr(const FString& v)
{
	return new KBVar(v);
}

bool DATATYPE_STRING::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::String || v.GetType() == EKBVarTypes::Ansichar;
}

KBVar* DATATYPE_VECTOR2::createFromStream(MemoryStream& stream)
{
	FVector2D val;
	stream >> val.X >> val.Y;
	return new KBVar(val);
}

void DATATYPE_VECTOR2::addToStream(Bundle& stream, KBVar& v)
{
	FVector2D val = v;
	stream << val.X << val.Y;
}

KBVar* DATATYPE_VECTOR2::parseDefaultValStr(const FString& v)
{
	return new KBVar(FVector2D());
}

bool DATATYPE_VECTOR2::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Vector2d;
}

KBVar* DATATYPE_VECTOR3::createFromStream(MemoryStream& stream)
{
	FVector val;
	stream >> val.X >> val.Y >> val.Z;
	return new KBVar(val);
}

void DATATYPE_VECTOR3::addToStream(Bundle& stream, KBVar& v)
{
	FVector val = v;
	stream << val.X << val.Y << val.Z;
}

KBVar* DATATYPE_VECTOR3::parseDefaultValStr(const FString& v)
{
	return new KBVar(FVector());
}

bool DATATYPE_VECTOR3::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Vector;
}

KBVar* DATATYPE_VECTOR4::createFromStream(MemoryStream& stream)
{
	FVector4 val;
	stream >> val.X >> val.Y >> val.Z >> val.W;
	return new KBVar(val);
}

void DATATYPE_VECTOR4::addToStream(Bundle& stream, KBVar& v)
{
	FVector4 val = v;
	stream << val.X << val.Y << val.Z << val.W;
}

KBVar* DATATYPE_VECTOR4::parseDefaultValStr(const FString& v)
{
	return new KBVar(FVector4());
}

bool DATATYPE_VECTOR4::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::Vector4;
}

KBVar* DATATYPE_PYTHON::createFromStream(MemoryStream& stream)
{
	TArray<uint8> val;
	stream.readBlob(val);
	return new KBVar(val);
}

void DATATYPE_PYTHON::addToStream(Bundle& stream, KBVar& v)
{
	TArray<uint8> val = v;
	stream.appendBlob(val);
}

KBVar* DATATYPE_PYTHON::parseDefaultValStr(const FString& v)
{
	return new KBVar(TArray<uint8>());
}

bool DATATYPE_PYTHON::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::ByteArray;
}

KBVar* DATATYPE_UNICODE::createFromStream(MemoryStream& stream)
{
	FString val;
	stream.readUTF8String(val);
	return new KBVar(val);
}

void DATATYPE_UNICODE::addToStream(Bundle& stream, KBVar& v)
{
	FString val = v;
	stream.appendUTF8String(val);
}

KBVar* DATATYPE_UNICODE::parseDefaultValStr(const FString& v)
{
	return new KBVar(v);
}

bool DATATYPE_UNICODE::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::String || v.GetType() == EKBVarTypes::Widechar
		|| v.GetType() == EKBVarTypes::Ansichar;
}

KBVar* DATATYPE_ENTITYCALL::createFromStream(MemoryStream& stream)
{
	TArray<uint8> val;
	stream.readBlob(val);
	return new KBVar(val);
}

void DATATYPE_ENTITYCALL::addToStream(Bundle& stream, KBVar& v)
{
	TArray<uint8> val = v;
	stream.appendBlob(val);
}

KBVar* DATATYPE_ENTITYCALL::parseDefaultValStr(const FString& v)
{
	return new KBVar(TArray<uint8>());
}

bool DATATYPE_ENTITYCALL::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::ByteArray;
}

KBVar* DATATYPE_BLOB::createFromStream(MemoryStream& stream)
{
	TArray<uint8> val;
	stream.readBlob(val);
	return new KBVar(val);
}

void DATATYPE_BLOB::addToStream(Bundle& stream, KBVar& v)
{
	TArray<uint8> val = v;
	stream.appendBlob(val);
}

KBVar* DATATYPE_BLOB::parseDefaultValStr(const FString& v)
{
	return new KBVar(TArray<uint8>());
}

bool DATATYPE_BLOB::isSameType(KBVar& v)
{
	return v.GetType() == EKBVarTypes::ByteArray;
}

void DATATYPE_ARRAY::bind()
{
	if (tmpset_uitemtype == -1)
		vtype->bind();
	else
		if (EntityDef::id2datatypes.Contains(tmpset_uitemtype))
			vtype = EntityDef::id2datatypes[tmpset_uitemtype];
}

KBVar* DATATYPE_ARRAY::createFromStream(MemoryStream& stream)
{
	KBVar::KBVarArray val;

	uint32 size;
	stream >> size;

	while (size > 0)
	{
		size--;
		val.Add(*vtype->createFromStream(stream));
	};

	return new KBVar(val);
}

void DATATYPE_ARRAY::addToStream(Bundle& stream, KBVar& v)
{
	if (v.GetType() == EKBVarTypes::KBVarArray)
	{
		KBVar::KBVarArray val = v;
		uint32 size = val.Num();

		stream << size;

		for (uint32 i = 0; i<size; ++i)
		{
			vtype->addToStream(stream, val[i]);
		}
	}
	else
	{
		KBVar::KBVarBytes val = v;
		uint32 size = val.Num();

		stream << size;

		for (uint32 i = 0; i<size; ++i)
		{
			KBVar tmpv = val[i];
			vtype->addToStream(stream, tmpv);
		}
	}
}

KBVar* DATATYPE_ARRAY::parseDefaultValStr(const FString& v)
{
	return new KBVar(KBVar::KBVarArray());
}

bool DATATYPE_ARRAY::isSameType(KBVar& v)
{
	if (v.GetType() != EKBVarTypes::KBVarArray && v.GetType() != EKBVarTypes::ByteArray)
		return false;

	if (v.GetType() == EKBVarTypes::KBVarArray)
	{
		KBVar::KBVarArray val = v;

		for (uint32 i = 0; i < (uint32)val.Num(); ++i)
		{
			if (!vtype->isSameType(val[i]))
				return false;
		}
	}
	else
	{
		KBVar::KBVarBytes val = v;

		for (uint32 i = 0; i < (uint32)val.Num(); ++i)
		{
			KBVar tmpv = val[i];
			if (!vtype->isSameType(tmpv))
				return false;
		}
	}

	return true;
}

void DATATYPE_FIXED_DICT::bind()
{
	if (dicttype_map.Num() > 0)
	{
		for (auto& item : dicttype_map)
		{
			if (EntityDef::id2datatypes.Contains(item.Value))
				dicttype.Add(item.Key, EntityDef::id2datatypes[item.Value]);
		}

		dicttype_map.Empty();
	}
	else
	{
		for (auto& item : dicttype)
		{
			item.Value->bind();
		}
	}
}

KBVar* DATATYPE_FIXED_DICT::createFromStream(MemoryStream& stream)
{
	KBVar::KBVarMap val;

	for (auto& item : dicttype)
	{
		val.Add(item.Key, *item.Value->createFromStream(stream));
	}

	return new KBVar(val);
}

void DATATYPE_FIXED_DICT::addToStream(Bundle& stream, KBVar& v)
{
	KBVar::KBVarMap val = v;

	for (auto& item : dicttype)
	{
		item.Value->addToStream(stream, val[item.Key]);
	}
}

KBVar* DATATYPE_FIXED_DICT::parseDefaultValStr(const FString& v)
{
	KBVar::KBVarMap val;

	for (auto& item : dicttype)
	{
		val.Add(item.Key, *item.Value->parseDefaultValStr(FString(TEXT(""))));
	}

	return new KBVar(val);
}

bool DATATYPE_FIXED_DICT::isSameType(KBVar& v)
{
	if (v.GetType() != EKBVarTypes::KBVarMap)
		return false;

	KBVar::KBVarMap val = v;

	for (auto& item : dicttype)
	{
		if (!item.Value->isSameType(val[item.Key]))
			return false;
	}

	return true;
}

}