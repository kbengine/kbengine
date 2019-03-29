// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Math/RandomStream.h"
#include "Misc/NetworkGuid.h"
#include "Misc/Variant.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

namespace EKBVarTypes
{
	const int32 Empty = 0;
	const int32 Ansichar = 1;
	const int32 Bool = 2;
	const int32 ByteArray = 3;
	const int32 Double = 4;
	const int32 Float = 5;
	const int32 Int8 = 6;
	const int32 Int16 = 7;
	const int32 Int32 = 8;
	const int32 Int64 = 9;
	const int32 String = 10;
	const int32 Widechar = 11;
	const int32 UInt8 = 12;
	const int32 UInt16 = 13;
	const int32 UInt32 = 14;
	const int32 UInt64 = 15;
	const int32 Vector = 16;
	const int32 Vector2d = 17;
	const int32 Vector4 = 18;
	const int32 KBVarArray = 19;
	const int32 KBVarMap = 20; /*TMap<FString, KBVar>*/
};

/**
* Stub for variant type traits.
*
* Actual type traits need to be declared through template specialization for custom
* data types that are to be used in KBVar. Traits for the most commonly used built-in
* types are declared below.
*
* Complex types, such as structures and classes can be serialized into a byte array
* and then assigned to a variant. Note that you will be responsible for ensuring
* correct byte ordering when serializing those types.
*
* @param T The type to be used in KBVar.
*/
namespace KBEngine
{

template<typename T> struct TKBVariantTraits
{
	static int32 GetType()
	{
		static_assert(!sizeof(T), "KBVar trait must be specialized for this type.");
		return (int32)EVariantTypes::Empty;
	}
};


/**
* Implements an extensible union of multiple types.
*
* Variant types can be used to store a range of different built-in types, as well as user
* defined types. The values are internally serialized into a byte array, which means that
* only FArchive serializable types are supported at this time.
*/
class KBENGINEPLUGINS_API KBVar
{
public:
	typedef TMap<FString, KBVar> KBVarMap;
	typedef TArray<KBVar> KBVarArray;
	typedef TArray<uint8> KBVarBytes;

	/** Default constructor. */
	KBVar()
		: Type(EKBVarTypes::Empty)
		, Error(0)
	{ }

	/** Copy constructor. */
	KBVar(const KBVar& Other)
		: Type(Other.Type)
		, Value(Other.Value)
		, Error(0)
	{ }

	/**
	* Creates and initializes a new instance with the specified value.
	*
	* @param InValue The initial value.
	*/
	template<typename T>
	KBVar(T InValue)
	{
		Error = 0;

		FMemoryWriter writer(Value, true);
		writer << InValue;

		Type = TKBVariantTraits<T>::GetType();
	}

	/**
	* Creates and initializes a new instance from a byte array.
	*
	* Array values are passed straight through as an optimization. Please note that, if you
	* serialize any complex types into arrays and then store them in KBVar, you will be
	* responsible for ensuring byte ordering if the KBVar gets sent over the network.
	*
	* @param InValue- The initial value.
	*/
	KBVar(const KBVarBytes& InArray)
		: Type(EKBVarTypes::ByteArray)
		, Value(InArray)
		, Error(0)
	{ }

	KBVar(const KBVarArray& InArray)
		: Type(EKBVarTypes::KBVarArray)
		, Value(),
		Error(0)
	{
		if (InArray.Num() > 0)
		{
			int idx = 0;
			for (auto& item : InArray)
			{
				KBVar& v = const_cast<KBVar&>(item);
				 
				int32 v_type = v.GetType();
				Value.SetNumUninitialized(idx + sizeof(v_type));
				memcpy(Value.GetData() + idx, (uint8*)(&v_type), sizeof(v_type));
				idx += sizeof(v_type);

				int size = v.GetSize();

				Value.SetNumUninitialized(idx + sizeof(size));
				memcpy(Value.GetData() + idx, (uint8*)(&size), sizeof(size));
				idx += sizeof(size);

				Value.SetNumUninitialized(idx + size);
				memcpy(Value.GetData() + idx, v.GetBytes().GetData(), size);
				idx += size;
			}
		}
	}

	KBVar(const KBVarMap& InMap)
		: Type(EKBVarTypes::KBVarMap)
		, Value()
		, Error(0)
	{
		int idx = 0;

		for (auto& item : InMap)
		{
			// FString key
			const FString& key = item.Key;
			int32 size = key.Len();

			Value.SetNumUninitialized(idx + sizeof(size));
			memcpy(Value.GetData() + idx, (uint8*)(&size), sizeof(size));
			idx += sizeof(size);

			Value.SetNumUninitialized(idx + size);
			memcpy(Value.GetData() + idx, (uint8*)(TCHAR_TO_UTF8(*key)), size);
			idx += size;

			// KBVar
			KBVar& v = const_cast<KBVar&>(item.Value);

			int32 v_type = v.GetType();
			Value.SetNumUninitialized(idx + sizeof(v_type));
			memcpy(Value.GetData() + idx, (uint8*)(&v_type), sizeof(v_type));
			idx += sizeof(v_type);

			size = v.GetSize();

			Value.SetNumUninitialized(idx + sizeof(size));
			memcpy(Value.GetData() + idx, (uint8*)(&size), sizeof(size));
			idx += sizeof(size);

			Value.SetNumUninitialized(idx + size);
			memcpy(Value.GetData() + idx, v.GetBytes().GetData(), size);
			idx += size;
		}
	}

	/**
	* Creates and initializes a new instance from a TCHAR string.
	*
	* @param InString The initial value.
	*/
	KBVar(const TCHAR* InString)
	{
		Error = 0;
		*this = FString(InString);
	}

	FString c_str()
	{
		if (GetType() == EKBVarTypes::Empty)
		{
			return FString::Printf(TEXT("Empty"));
		}
		else if (GetType() == EKBVarTypes::Ansichar)
		{
			return *this;
		}
		else if (GetType() == EKBVarTypes::Bool)
		{
			bool v = (*this);
			return FString::Printf(TEXT("%s"), (v ? "true" : "false"));
		}
		else if (GetType() == EKBVarTypes::ByteArray)
		{
			TArray<uint8> v = (*this);
			return FString::Printf(TEXT("ByteArray(size=%d)"), v.Num());
		}
		else if (GetType() == EKBVarTypes::Double)
		{
			double v = (*this);
			return FString::Printf(TEXT("%lf"), v);
		}
		else if (GetType() == EKBVarTypes::Float)
		{
			float v = (*this);
			return FString::Printf(TEXT("%f"), v);
		}
		else if (GetType() == EKBVarTypes::Int8)
		{
			int8 v = (*this);
			return FString::Printf(TEXT("%d"), (int)v);
		}
		else if (GetType() == EKBVarTypes::Int16)
		{
			int16 v = (*this);
			return FString::Printf(TEXT("%d"), (int)v);
		}
		else if (GetType() == EKBVarTypes::Int32)
		{
			int32 v = (*this);
			return FString::Printf(TEXT("%d"), v);
		}
		else if (GetType() == EKBVarTypes::Int64)
		{
			int64 v = (*this);
			return FString::Printf(TEXT("%lld"), v);
		}
		else if (GetType() == EKBVarTypes::String)
		{
			return *this;
		}
		else if (GetType() == EKBVarTypes::Widechar)
		{
			return *this;
		}
		else if (GetType() == EKBVarTypes::UInt8)
		{
			uint8 v = (*this);
			return FString::Printf(TEXT("%d"), (int)v);
		}
		else if (GetType() == EKBVarTypes::UInt16)
		{
			uint16 v = (*this);
			return FString::Printf(TEXT("%d"), (int)v);
		}
		else if (GetType() == EKBVarTypes::UInt32)
		{
			uint32 v = (*this);
			return FString::Printf(TEXT("%ld"), v);
		}
		else if (GetType() == EKBVarTypes::UInt64)
		{
			uint64 v = (*this);
			return FString::Printf(TEXT("%lld"), v);
		}
		else if (GetType() == EKBVarTypes::Vector2d)
		{
			FVector2D v = (*this);
			return FString::Printf(TEXT("Vector2d(%f, %f)"), v.X, v.Y);
		}
		else if (GetType() == EKBVarTypes::Vector)
		{
			FVector v = (*this);
			return FString::Printf(TEXT("Vector(%f, %f, %f)"), v.X, v.Y, v.Z);
		}
		else if (GetType() == EKBVarTypes::Vector4)
		{
			FVector4 v = (*this);
			return FString::Printf(TEXT("Vector(%f, %f, %f, %f)"), v.X, v.Y, v.Z, v.W);
		}
		else if (GetType() == EKBVarTypes::KBVarArray)
		{
			return FString::Printf(TEXT("KBVarArray"));
		}
		else if (GetType() == EKBVarTypes::KBVarMap)
		{
			return FString::Printf(TEXT("KBVarMap"));
		}

		return FString();
	}

	FString type_str(int32 t) const
	{
		if (t == EKBVarTypes::Empty)
		{
			return FString(TEXT("Empty"));
		}
		else if (t == EKBVarTypes::Ansichar)
		{
			return FString(TEXT("String"));
		}
		else if (t == EKBVarTypes::Bool)
		{
			return FString(TEXT("Bool"));
		}
		else if (t == EKBVarTypes::ByteArray)
		{
			return FString(TEXT("ByteArray"));
		}
		else if (t == EKBVarTypes::Double)
		{
			return FString(TEXT("Double"));
		}
		else if (t == EKBVarTypes::Float)
		{
			return FString(TEXT("Float"));
		}
		else if (t == EKBVarTypes::Int8)
		{
			return FString(TEXT("Int8"));
		}
		else if (t == EKBVarTypes::Int16)
		{
			return FString(TEXT("Int16"));
		}
		else if (t == EKBVarTypes::Int32)
		{
			return FString(TEXT("Int32"));
		}
		else if (t == EKBVarTypes::Int64)
		{
			return FString(TEXT("Int64"));
		}
		else if (t == EKBVarTypes::String)
		{
			return FString(TEXT("String"));
		}
		else if (t == EKBVarTypes::Widechar)
		{
			return FString(TEXT("String"));
		}
		else if (t == EKBVarTypes::UInt8)
		{
			return FString(TEXT("UInt8"));
		}
		else if (t == EKBVarTypes::UInt16)
		{
			return FString(TEXT("UInt16"));
		}
		else if (t == EKBVarTypes::UInt32)
		{
			return FString(TEXT("UInt32"));
		}
		else if (t == EKBVarTypes::UInt64)
		{
			return FString(TEXT("UInt64"));
		}
		else if (t == EKBVarTypes::Vector2d)
		{
			return FString(TEXT("Vector2d"));
		}
		else if (t == EKBVarTypes::Vector)
		{
			return FString(TEXT("Vector"));
		}
		else if (t == EKBVarTypes::Vector4)
		{
			return FString(TEXT("Vector4"));
		}
		else if (t == EKBVarTypes::KBVarArray)
		{
			return FString(TEXT("KBVarArray"));
		}
		else if (t == EKBVarTypes::KBVarMap)
		{
			return FString(TEXT("KBVarMap"));
		}

		return FString(TEXT("UNKNOWN"));
	}

public:

	/**
	* Copy assignment operator.
	*
	* @param Other The value to assign.
	* @return This instance.
	*/
	KBVar& operator=(const KBVar& Other)
	{
		if (&Other != this)
		{
			Value = Other.Value;
			Type = Other.Type;
		}

		return *this;
	}

	/**
	* Assignment operator.
	*
	* @param T The type of the value to assign.
	* @param InValue The value to assign.
	* @return This instance.
	*/
	template<typename T>
	KBVar& operator=(T InValue)
	{
		FMemoryWriter Writer(Value, true);
		Writer << InValue;

		Type = TKBVariantTraits<T>::GetType();

		return *this;
	}

	/**
	* Assignment operator for byte arrays.
	*
	* Array values are passed straight through as an optimization. Please note that, if you
	* serialize any complex types into arrays and then store them in KBVar, you will be
	* responsible for ensuring byte ordering if the KBVar gets sent over the network.
	*
	* @param InArray The byte array to assign.
	* @return This instance.
	*/
	KBVar& operator=(const KBVarBytes& InArray)
	{
		Type = EKBVarTypes::ByteArray;
		Value = MoveTemp(const_cast<KBVarBytes&>(InArray));

		return *this;
	}

	/**
	* Assignment operator for TCHAR strings.
	*
	* @param InString The value to assign.
	* @return This instance.
	*/
	KBVar& operator=(const TCHAR* InString)
	{
		*this = FString(InString);

		return *this;
	}


	/**
	* Implicit conversion operator.
	*
	* @param T The type to convert the value to.
	* @return The value converted to the specified type.
	*/
	template<typename T>
	operator T() const
	{
		return GetValue<T>();
	}

public:

	/**
	* Comparison operator for equality.
	*
	* @param Other The variant to compare with.
	* @return true if the values are equal, false otherwise.
	*/
	bool operator==(const KBVar& Other) const
	{
		return ((Type == Other.Type) && (Value == Other.Value));
	}

	/**
	* Comparison operator for inequality.
	*
	* @param Other The variant to compare with.
	* @return true if the values are not equal, false otherwise.
	*/
	bool operator!=(const KBVar& Other) const
	{
		return ((Type != Other.Type) || (Value != Other.Value));
	}

public:

	/**
	* Empties the value.
	*
	* @see IsEmpty
	*/
	void Empty()
	{
		Type = EKBVarTypes::Empty;

		Value.Empty();
	}

	/**
	* Checks whether the value is empty.
	*
	* @return true if the value is empty, false otherwise.
	*
	* @see Empty
	*/
	bool IsEmpty() const
	{
		return (Type == EKBVarTypes::Empty);
	}

	/**
	* Gets the stored value as a byte array.
	*
	* This method returns the internal representation of any value as an
	* array of raw bytes. To retrieve values of type TArray<uint8> use
	* GetValue<TArray<uint8>>() instead.
	*
	* @return Byte array.
	* @see GetValue
	*/
	KBVar::KBVarBytes& GetBytes()
	{
		return Value;
	}

	/**
	* Gets the stored value's size (in bytes).
	*
	* @return Size of the value.
	* @see GetType, GetValue
	*/
	int32 GetSize() const
	{
		return Value.Num();
	}

	/**
	* Gets the stored value's type.
	*
	* @return Type of the value.
	* @see GetSize, GetValue
	*/
	int32 GetType() const
	{
		return Type;
	}

	void SetType(int32 t) 
	{
		Type = t;
	}

	uint8 GetError() const
	{
		return Error;
	}

	void ErrorLog(const FString& errstr) const;

	/**
	* Gets the stored value.
	*
	* This template function does not provide any automatic conversion between
	* convertible types. The exact type of the value to be extracted must be known.
	*
	* @return The value.
	* @see GetSize, GetType
	*/
	template<typename T>
	T GetValue() const
	{
		if (!((Type == TKBVariantTraits<T>::GetType()) || ((TKBVariantTraits<T>::GetType() == EKBVarTypes::UInt8))))
		{
			ErrorLog(FString::Printf(TEXT("KBVar::GetValue<T>(): Type mismatch! The current is %s, given the %s"), *type_str(Type), *type_str(TKBVariantTraits<T>::GetType())));
		}

		T Result;

		FMemoryReader Reader(Value, true);
		Reader << Result;

		return Result;
	}

public:

	/**
	* Serializes the given variant type from or into the specified archive.
	*
	* @param Ar The archive to serialize from or into.
	* @param Variant The value to serialize.
	* @return The archive.
	*/
	friend FArchive& operator<<(FArchive& Ar, KBVar& Variant)
	{
		return Ar << Variant.Type << Variant.Value;
	}

private:

	/** Holds the type of the variant. */
	int32 Type;

	/** Holds the serialized value. */
	KBVar::KBVarBytes Value;

	uint8 Error;
};


/**
* Gets the stored value for byte arrays.
*
* Array values are passed straight through as an optimization. Please note that, if you serialize
* any complex types into arrays and then store them in KBVar, you will be responsible for
* ensuring byte ordering if the KBVar gets send over the network.
*
* To retrieve any value as an array of serialized bytes, use GetBytes() instead.
*
* @return The byte array.
* @see GetBytes
*/
template<>
FORCEINLINE KBVar::KBVarBytes KBVar::GetValue<KBVar::KBVarBytes >() const
{
	if (Type != EKBVarTypes::ByteArray)
	{
		ErrorLog(FString::Printf(TEXT("KBVar::GetValue<KBVar::KBVarBytes>(): Type mismatch! The current is %s, given the KBVar::KBVarBytes"), *type_str(Type)));
		return KBVar::KBVarBytes();
	}

	return Value;
}

template<>
FORCEINLINE KBVar::KBVarArray KBVar::GetValue<KBVar::KBVarArray >() const
{
	if (Type != EKBVarTypes::KBVarArray)
	{
		ErrorLog(FString::Printf(TEXT("KBVar::GetValue<KBVar::KBVarArray>(): Type mismatch! The current is %s, given the KBVar::KBVarArray"), *type_str(Type)));
		return KBVar::KBVarArray();
	}

	KBVar::KBVarArray v_array;
	int idx = 0;

	while (idx < Value.Num())
	{
		int32 vtype = *((int32*)(Value.GetData() + idx));
		idx += sizeof(vtype);

		int size = *((int*)(Value.GetData() + idx));
		idx += sizeof(size);

		KBVar v;
		v.SetType(vtype);

		v.GetBytes().SetNumUninitialized(size);
		memcpy(v.GetBytes().GetData(), Value.GetData() + idx, size);
		idx += size;

		v_array.Add(v);
	}

	return v_array;
}

template<>
FORCEINLINE KBVar::KBVarMap KBVar::GetValue<KBVar::KBVarMap >() const
{
	if (Type != EKBVarTypes::KBVarMap)
	{
		ErrorLog(FString::Printf(TEXT("KBVar::GetValue<KBVar::KBVarMap>(): Type mismatch! The current is %s, given the KBVar::KBVarMap"), *type_str(Type)));
		return KBVar::KBVarMap();
	}

	KBVar::KBVarMap v_map;
	int idx = 0;

	while (idx < Value.Num())
	{
		int32 size = *((int*)(Value.GetData() + idx));
		idx += sizeof(size);

		TArray<uint8> key_datas;
		key_datas.SetNumUninitialized(size);
		memcpy(key_datas.GetData(), Value.GetData() + idx, size);
		key_datas.Add(0);
		FString key = FString(UTF8_TO_TCHAR(key_datas.GetData()));
		idx += size;

		int32 vtype = *((int32*)(Value.GetData() + idx));
		idx += sizeof(vtype);

		size = *((int32*)(Value.GetData() + idx));
		idx += sizeof(size);

		KBVar v;
		v.SetType(vtype);

		v.GetBytes().SetNumUninitialized(size);
		memcpy(v.GetBytes().GetData(), Value.GetData() + idx, size);
		idx += size;

		v_map.Add(key, v);
	}

	return v_map;
}


/* Default KBVar traits for built-in types
*****************************************************************************/

/** Implements variant type traits for the built-in ANSICHAR type. */
template<> struct TKBVariantTraits<ANSICHAR>
{
	static int32 GetType() { return EKBVarTypes::Ansichar; }
};


/** Implements variant type traits for the built-in bool type. */
template<> struct TKBVariantTraits<bool>
{
	static int32 GetType() { return EKBVarTypes::Bool; }
};


/** Implements variant type traits for byte arrays. */
template<> struct TKBVariantTraits<TArray<uint8> >
{
	static int32 GetType() { return EKBVarTypes::ByteArray; }
};


/** Implements variant type traits for the built-in double type. */
template<> struct TKBVariantTraits<double>
{
	static int32 GetType() { return EKBVarTypes::Double; }
};


/** Implements variant type traits for the built-in float type. */
template<> struct TKBVariantTraits<float>
{
	static int32 GetType() { return EKBVarTypes::Float; }
};


/** Implements variant type traits for the built-in int8 type. */
template<> struct TKBVariantTraits<int8>
{
	static int32 GetType() { return EKBVarTypes::Int8; }
};


/** Implements variant type traits for the built-in int16 type. */
template<> struct TKBVariantTraits<int16>
{
	static int32 GetType() { return EKBVarTypes::Int16; }
};


/** Implements variant type traits for the built-in int32 type. */
template<> struct TKBVariantTraits<int32>
{
	static int32 GetType() { return EKBVarTypes::Int32; }
};


/** Implements variant type traits for the built-in int64 type. */
template<> struct TKBVariantTraits<int64>
{
	static int32 GetType() { return EKBVarTypes::Int64; }
};


/** Implements variant type traits for the built-in FString type. */
template<> struct TKBVariantTraits<FString>
{
	static int32 GetType() { return EKBVarTypes::String; }
};


/** Implements variant type traits for the built-in WIDECHAR type. */
template<> struct TKBVariantTraits<WIDECHAR>
{
	static int32 GetType() { return EKBVarTypes::Widechar; }
};


/** Implements variant type traits for the built-in uint8 type. */
template<> struct TKBVariantTraits<uint8>
{
	static int32 GetType() { return EKBVarTypes::UInt8; }
};


/** Implements variant type traits for the built-in uint16 type. */
template<> struct TKBVariantTraits<uint16>
{
	static int32 GetType() { return EKBVarTypes::UInt16; }
};


/** Implements variant type traits for the built-in uint32 type. */
template<> struct TKBVariantTraits<uint32>
{
	static int32 GetType() { return EKBVarTypes::UInt32; }
};


/** Implements variant type traits for the built-in uint64 type. */
template<> struct TKBVariantTraits<uint64>
{
	static int32 GetType() { return EKBVarTypes::UInt64; }
};


/** Implements variant type traits for the built-in FVector type. */
template<> struct TKBVariantTraits<FVector>
{
	static int32 GetType() { return EKBVarTypes::Vector; }
};


/** Implements variant type traits for the built-in FVector2D type. */
template<> struct TKBVariantTraits<FVector2D>
{
	static int32 GetType() { return EKBVarTypes::Vector2d; }
};


/** Implements variant type traits for the built-in FVector4 type. */
template<> struct TKBVariantTraits<FVector4>
{
	static int32 GetType() { return EKBVarTypes::Vector4; }
};


/** Implements variant type traits for KBVar arrays. */
template<> struct TKBVariantTraits<TArray<KBVar>>
{
	static int32 GetType() { return EKBVarTypes::KBVarArray; }
};


/** Implements variant type traits for KBVar maps. */
template<> struct TKBVariantTraits<TMap<FString, KBVar>>
{
	static int32 GetType() { return EKBVarTypes::KBVarMap; }
};

}