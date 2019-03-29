// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"
#include "KBDebug.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformProperties.h"

namespace MemoryStreamConverter
{
	template <class T> 
	void swap(T& a, T& b)
	{
		T c(a); a = b; b = c;
	}

	template<size_t T>
	inline void convert(char *val)
	{
		swap(*val, *(val + T - 1));
		convert<T - 2>(val + 1);
	}

	template<> inline void convert<0>(char *) {}
	template<> inline void convert<1>(char *) {}            // ignore central byte

	template<typename T> inline void apply(T *val)
	{
		convert<sizeof(T)>((char *)(val));
	}

	inline void convert(char *val, size_t size)
	{
		if (size < 2)
			return;

		swap(*val, *(val + size - 1));
		convert(val + 1, size - 2);
	}
}

namespace KBEngine
{

template<typename T> inline void EndianConvert(T& val) 
{ 
	if(!FGenericPlatformProperties::IsLittleEndian())
		MemoryStreamConverter::apply<T>(&val); 
}

template<typename T> inline void EndianConvertReverse(T& val) 
{
	if (FGenericPlatformProperties::IsLittleEndian())
		MemoryStreamConverter::apply<T>(&val);
}

template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error

inline void EndianConvert(uint8&) { }
inline void EndianConvert(int8&) { }
inline void EndianConvertReverse(uint8&) { }
inline void EndianConvertReverse(int8&) { }

/*
	二进制数据流模块
	能够将一些基本类型序列化(writeXXX)成二进制流同时也提供了反序列化(readXXX)等操作
*/
class KBENGINEPLUGINS_API MemoryStream
{
public:
	union PackFloatXType
	{
		float	fv;
		uint32	uv;
		int		iv;
	};

public:
	const static size_t DEFAULT_SIZE = 0x100;
	const static size_t MAX_SIZE = 10000000;

	MemoryStream() :
		rpos_(0), 
		wpos_(0),
		data_(new TArray<uint8>)
	{
		data_resize(DEFAULT_SIZE);
	}

	virtual ~MemoryStream()
	{
		clear(false);
		KBE_SAFE_RELEASE(data_);
	}

	static MemoryStream* createObject();
	static void reclaimObject(MemoryStream* obj);

public:
	uint8* data() {
		return data_->GetData();
	}

	void clear(bool clearData)
	{
		if (clearData)
			data_->Empty();

		rpos_ = wpos_ = 0;
	}

	// array的大小
	virtual uint32 size() const { return data_->Num(); }

	// array是否为空
	virtual bool empty() const { return data_->Num() == 0; }

	// 读索引到与写索引之间的长度
	virtual uint32 length() const { return rpos() >= wpos() ? 0 : wpos() - rpos(); }

	// 剩余可填充的大小
	virtual uint32 space() const { return wpos() >= size() ? 0 : size() - wpos(); }

	// 将读索引强制设置到写索引，表示操作结束
	void done() { read_skip(length()); }

	void data_resize(uint32 newsize)
	{
		KBE_ASSERT(newsize <= MAX_SIZE);
		data_->SetNumUninitialized(newsize);
	}

	void resize(uint32 newsize)
	{
		KBE_ASSERT(newsize <= MAX_SIZE);
		data_->SetNumUninitialized(newsize);
		rpos_ = 0;
		wpos_ = size();
	}

	void reserve(uint32 ressize)
	{
		KBE_ASSERT(ressize <= MAX_SIZE);

		if (ressize > size())
			data_->Reserve(ressize);
	}

	uint32 rpos() const { return rpos_; }

	uint32 rpos(int rpos)
	{
		if (rpos < 0)
			rpos = 0;

		rpos_ = rpos;
		return rpos_;
	}

	uint32 wpos() const { return wpos_; }

	uint32 wpos(int wpos)
	{
		if (wpos < 0)
			wpos = 0;

		wpos_ = wpos;
		return wpos_;
	}

	void swap(MemoryStream & s)
	{
		size_t rpos = s.rpos(), wpos = s.wpos();

		TArray<uint8> *temp = data_;
		data_ = s.data_;
		s.data_ = temp;

		s.rpos((int)rpos_);
		s.wpos((int)wpos_);
		rpos_ = rpos;
		wpos_ = wpos;
	}

	uint8 operator[](uint32 pos)
	{
		return read<uint8>(pos);
	}

	MemoryStream &operator<<(uint8 value)
	{
		append<uint8>(value);
		return *this;
	}

	MemoryStream &operator<<(uint16 value)
	{
		append<uint16>(value);
		return *this;
	}

	MemoryStream &operator<<(uint32 value)
	{
		append<uint32>(value);
		return *this;
	}

	MemoryStream &operator<<(uint64 value)
	{
		append<uint64>(value);
		return *this;
	}

	MemoryStream &operator<<(int8 value)
	{
		append<int8>(value);
		return *this;
	}

	MemoryStream &operator<<(int16 value)
	{
		append<int16>(value);
		return *this;
	}

	MemoryStream &operator<<(int32 value)
	{
		append<int32>(value);
		return *this;
	}

	MemoryStream &operator<<(int64 value)
	{
		append<int64>(value);
		return *this;
	}

	MemoryStream &operator<<(float value)
	{
		append<float>(value);
		return *this;
	}

	MemoryStream &operator<<(double value)
	{
		append<double>(value);
		return *this;
	}

	MemoryStream &operator<<(const FString &value)
	{
		const TCHAR *serializedChar = value.GetCharArray().GetData();
		
		if(!serializedChar)
		{
			append((uint8)0);
			return *this;
		}

		uint32 size = FCString::Strlen(serializedChar);

		append(((uint8*)TCHAR_TO_ANSI(serializedChar)), size);
		append((uint8)0);
		return *this;
	}

	MemoryStream &operator<<(const char *str)
	{
		append((uint8 const *)str, str ? strlen(str) : 0);
		append((uint8)0);
		return *this;
	}

	MemoryStream &operator<<(bool value)
	{
		append<int8>(value);
		return *this;
	}

	MemoryStream &operator>>(bool &value)
	{
		value = read<char>() > 0 ? true : false;
		return *this;
	}

	MemoryStream &operator>>(uint8 &value)
	{
		value = read<uint8>();
		return *this;
	}

	MemoryStream &operator>>(uint16 &value)
	{
		value = read<uint16>();
		return *this;
	}

	MemoryStream &operator>>(uint32 &value)
	{
		value = read<uint32>();
		return *this;
	}

	MemoryStream &operator>>(uint64 &value)
	{
		value = read<uint64>();
		return *this;
	}

	MemoryStream &operator>>(int8 &value)
	{
		value = read<int8>();
		return *this;
	}

	MemoryStream &operator>>(int16 &value)
	{
		value = read<int16>();
		return *this;
	}

	MemoryStream &operator>>(int32 &value)
	{
		value = read<int32>();
		return *this;
	}

	MemoryStream &operator>>(int64 &value)
	{
		value = read<int64>();
		return *this;
	}

	MemoryStream &operator>>(float &value)
	{
		value = read<float>();
		return *this;
	}

	MemoryStream &operator>>(double &value)
	{
		value = read<double>();
		return *this;
	}

	MemoryStream &operator>>(FString& value)
	{
		char cc[2];
		cc[0] = 0;
		cc[1] = 0;

		value = TEXT("");
		while (length() > 0)
		{
			cc[0] = read<char>();
			if (cc[0] == 0 || !isascii(cc[0]))
				break;

			value += ANSI_TO_TCHAR((const ANSICHAR*)&cc);
		}

		return *this;
	}

	MemoryStream &operator>>(char *value)
	{
		while (length() > 0)
		{
			char c = read<char>();
			if (c == 0 || !isascii(c))
				break;

			*(value++) = c;
		}

		*value = '\0';
		return *this;
	}

	template<typename T>
	void read_skip() { read_skip(sizeof(T)); }

	void read_skip(uint32 skip)
	{
		check(skip <= length());
		rpos_ += skip;
	}

	template <typename T> T read()
	{
		T r = read<T>(rpos_);
		rpos_ += sizeof(T);
		return r;
	}

	template <typename T> T read(uint32 pos) 
	{
		check(sizeof(T) <= length());

		T val;
		memcpy((void*)&val, (data() + pos), sizeof(T));

		EndianConvert(val);
		return val;
	}

	void read(uint8 *dest, uint32 len)
	{
		check(len <= length());

		memcpy(dest, data() + rpos_, len);
		rpos_ += len;
	}

	int8 readInt8()
	{
		return read<int8>();
	}

	int16 readInt16()
	{
		return read<int16>();
	}
		
	int32 readInt32()
	{
		return read<int32>();
	}

	int64 readInt64()
	{
		return read<int64>();
	}
	
	uint8 readUint8()
	{
		return read<uint8>();
	}

	uint16 readUint16()
	{
		return read<uint16>();
	}

	uint32 readUint32()
	{
		return read<uint32>();
	}
	
	uint64 readUint64()
	{
		return read<uint64>();
	}
	
	float readFloat()
	{
		return read<float>();
	}

	double readDouble()
	{
		return read<double>();
	}
	
	TArray<uint8> readBlob()
	{
		TArray<uint8> datas;
		readBlob(datas);
		return datas;
	}

	uint32 readBlob(TArray<uint8>& datas)
	{
		if (length() <= 0)
			return 0;

		uint32 rsize = 0;
		(*this) >> rsize;
		if ((uint32)rsize > length())
			return 0;

		if (rsize > 0)
		{
			datas.SetNumUninitialized(rsize);
			memcpy(datas.GetData(), data() + rpos(), rsize);
			read_skip(rsize);
		}

		return rsize;
	}

	uint32 readUTF8String(FString& str)
	{
		if (length() <= 0)
			return 0;

		TArray<uint8> datas;
		uint32 rsize = readBlob(datas);

		// 结尾标志
		datas.Add(0);

		str = FString(UTF8_TO_TCHAR(datas.GetData()));
		return rsize;
	}

	FString readUnicode()
	{
		FString str;
		readUTF8String(str);
		return str;
	}
	
	FString readString()
	{
		FString str;
		(*this) >> str;
		return str;
	}

	TArray<uint8> readEntitycall()
	{
		TArray<uint8> datas;
		uint64 cid = readUint64();
		int32 id = readInt32();
		uint16 type = readUint16();
		uint16 utype = readUint16();
		return datas;
	}

	uint32 readEntitycall(TArray<uint8>& datas)
	{
		if (length() <= 0)
			return 0;

		uint64 cid = readUint64();
		int32 id = readInt32();
		uint16 type = readUint16();
		uint16 utype = readUint16();
		return 0;
	}

	FVector2D readVector2()
	{
		float X = readFloat();
		float Y = readFloat();
		return FVector2D(X, Y);
	}

	FVector readVector3()
	{
		float X = readFloat();
		float Y = readFloat();
		float Z = readFloat();
		return FVector(X, Y, Z);
	}

	FVector4 readVector4()
	{
		float X = readFloat();
		float Y = readFloat();
		float Z = readFloat();
		float W = readFloat();
		return FVector4(X, Y, Z, W);
	}

	TArray<uint8> readPython()
	{
		return readBlob();
	}

	void readPackXYZ(float& x, float&y, float& z, float minf = -256.f)
	{
		uint32 packed = 0;
		(*this) >> packed;
		x = ((packed & 0x7FF) << 21 >> 21) * 0.25f;
		z = ((((packed >> 11) & 0x7FF) << 21) >> 21) * 0.25f;
		y = ((packed >> 22 << 22) >> 22) * 0.25f;

		x += minf;
		y += minf / 2.f;
		z += minf;
	}

	void readPackXZ(float& x, float& z)
	{
		PackFloatXType & xPackData = (PackFloatXType&)x;
		PackFloatXType & zPackData = (PackFloatXType&)z;

		// 0x40000000 = 1000000000000000000000000000000.
		xPackData.uv = 0x40000000;
		zPackData.uv = 0x40000000;

		uint8 tv;
		uint32 data = 0;

		(*this) >> tv;
		data |= (tv << 16);

		(*this) >> tv;
		data |= (tv << 8);

		(*this) >> tv;
		data |= tv;

		// 复制指数和尾数
		xPackData.uv |= (data & 0x7ff000) << 3;
		zPackData.uv |= (data & 0x0007ff) << 15;

		xPackData.fv -= 2.0f;
		zPackData.fv -= 2.0f;

		// 设置标记位
		xPackData.uv |= (data & 0x800000) << 8;
		zPackData.uv |= (data & 0x000800) << 20;
	}

	void readPackY(float& y)
	{
		PackFloatXType yPackData;
		yPackData.uv = 0x40000000;

		uint16 data = 0;
		(*this) >> data;
		yPackData.uv |= (data & 0x7fff) << 12;
		yPackData.fv -= 2.f;
		yPackData.uv |= (data & 0x8000) << 16;
		y = yPackData.fv;
	}

	template <typename T> void append(T value)
	{
		EndianConvert(value);
		append((uint8 *)&value, sizeof(value));
	}

	template<class T> void append(const T *src, uint32 cnt)
	{
		return append((const uint8 *)src, cnt * sizeof(T));
	}

	void append(const uint8 *src, uint32 cnt)
	{
		if (!cnt)
			return;

		KBE_ASSERT(size() < 10000000);

		if (size() < wpos_ + cnt)
			data_resize(wpos_ + cnt);

		memcpy((void*)&data()[wpos_], src, cnt);
		wpos_ += cnt;
	}

	void append(const uint8* datas, uint32 offset, uint32 size)
	{
		append(datas + offset, size);
	}

	void append(MemoryStream& stream)
	{
		wpos(stream.wpos());
		rpos(stream.rpos());
		data_resize(stream.size());
		memcpy(data(), stream.data(), stream.size());
	}

	void appendBlob(const TArray<uint8>& datas)
	{
		uint32 len = (uint32)datas.Num();
		(*this) << len;

		if (len > 0)
			append(datas.GetData(), len);
	}

	void appendUTF8String(const FString& str)
	{
		FTCHARToUTF8 EchoStrUtf8(*str);
		uint32 len = (uint32)EchoStrUtf8.Length();
		(*this) << len;

		if (len > 0)
			append(TCHAR_TO_UTF8(*str), len);
	}

	void appendPackAnyXYZ(float x, float y, float z, const float epsilon = 0.5f)
	{
		if (epsilon > 0.f)
		{
			x = floorf(x + epsilon);
			y = floorf(y + epsilon);
			z = floorf(z + epsilon);
		}

		*this << x << y << z;
	}

	void appendPackAnyXZ(float x, float z, const float epsilon = 0.5f)
	{
		if (epsilon > 0.f)
		{
			x = floorf(x + epsilon);
			z = floorf(z + epsilon);
		}

		*this << x << z;
	}

	void appendPackXYZ(float x, float y, float z, float minf = -256.f)
	{
		x -= minf;
		y -= minf / 2.f;
		z -= minf;

		// 最大值不要超过-256~256
		// y 不要超过-128~128
		uint32 packed = 0;
		packed |= ((int)(x / 0.25f) & 0x7FF);
		packed |= ((int)(z / 0.25f) & 0x7FF) << 11;
		packed |= ((int)(y / 0.25f) & 0x3FF) << 22;
		*this << packed;
	}

	void appendPackXZ(float x, float z)
	{
		PackFloatXType xPackData;
		xPackData.fv = x;

		PackFloatXType zPackData;
		zPackData.fv = z;

		// 0-7位存放尾数, 8-10位存放指数, 11位存放标志
		// 由于使用了24位来存储2个float， 并且要求能够达到-512~512之间的数
		// 8位尾数只能放最大值256, 指数只有3位(决定浮点数最大值为2^(2^3)=256) 
		// 我们舍去第一位使范围达到(-512~-2), (2~512)之间
		// 因此这里我们保证最小数为-2.f或者2.f
		xPackData.fv += xPackData.iv < 0 ? -2.f : 2.f;
		zPackData.fv += zPackData.iv < 0 ? -2.f : 2.f;

		uint32 data = 0;

		// 0x7ff000 = 11111111111000000000000
		// 0x0007ff = 00000000000011111111111
		const uint32 xCeilingValues[] = { 0, 0x7ff000 };
		const uint32 zCeilingValues[] = { 0, 0x0007ff };

		// 这里如果这个浮点数溢出了则设置浮点数为最大数
		// 这里检查了指数高4位和标记位， 如果高四位不为0则肯定溢出， 如果低4位和8位尾数不为0则溢出
		// 0x7c000000 = 1111100000000000000000000000000
		// 0x40000000 = 1000000000000000000000000000000
		// 0x3ffc000  = 0000011111111111100000000000000
		data |= xCeilingValues[((xPackData.uv & 0x7c000000) != 0x40000000) || ((xPackData.uv & 0x3ffc000) == 0x3ffc000)];
		data |= zCeilingValues[((zPackData.uv & 0x7c000000) != 0x40000000) || ((zPackData.uv & 0x3ffc000) == 0x3ffc000)];

		// 复制8位尾数和3位指数， 如果浮点数剩余尾数最高位是1则+1四舍五入, 并且存放到data中
		// 0x7ff000 = 11111111111000000000000
		// 0x0007ff = 00000000000011111111111
		// 0x4000	= 00000000100000000000000
		data |= ((xPackData.uv >> 3) & 0x7ff000) + ((xPackData.uv & 0x4000) >> 2);
		data |= ((zPackData.uv >> 15) & 0x0007ff) + ((zPackData.uv & 0x4000) >> 14);

		// 确保值在范围内
		// 0x7ff7ff = 11111111111011111111111
		data &= 0x7ff7ff;

		// 复制标记位
		// 0x800000 = 100000000000000000000000
		// 0x000800 = 000000000000100000000000
		data |= (xPackData.uv >> 8) & 0x800000;
		data |= (zPackData.uv >> 20) & 0x000800;

		uint8 packs[3];
		packs[0] = (uint8)(data >> 16);
		packs[1] = (uint8)(data >> 8);
		packs[2] = (uint8)data;
		(*this).append(packs, 3);
	}

	void appendPackY(float y)
	{
		PackFloatXType yPackData;
		yPackData.fv = y;

		yPackData.fv += yPackData.iv < 0 ? -2.f : 2.f;
		uint16 data = 0;
		data = (yPackData.uv >> 12) & 0x7fff;
		data |= ((yPackData.uv >> 16) & 0x8000);

		(*this) << data;
	}

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
	
	void writeBlob(const TArray<uint8>& datas)
	{
		appendBlob(datas);
	}
		
	void writeString(const FString& v)
	{
		if(v.Len() > (int32)space())
		{
			ERROR_MSG("MemoryStream::writeString: no free!");
			return;
		}

		(*this) << v;
	}

	void writeEntitycall(const TArray<uint8>& v)
	{
		uint64 cid = 0;
		int32 id = 0;
		uint16 type = 0;
		uint16 utype = 0;

		(*this) << cid << id << type << utype;
	}

	void writeVector2(const FVector2D& v)
	{
		writeFloat(v.X);
		writeFloat(v.Y);
	}

	void writeVector3(const FVector& v)
	{
		writeFloat(v.X);
		writeFloat(v.Y);
		writeFloat(v.Z);
	}

	void writeVector4(const FVector4& v)
	{
		writeFloat(v.X);
		writeFloat(v.Y);
		writeFloat(v.Z);
		writeFloat(v.W);
	}

	/** 输出流数据 */
	void print_storage();
	void hexlike();

protected:
	uint32 rpos_;
	uint32 wpos_;

	TArray<uint8>* data_;
};

template<>
inline void MemoryStream::read_skip<char*>()
{
	uint8 temp = 1;
	while (temp != 0)
		temp = read<uint8>();
}

template<>
inline void MemoryStream::read_skip<char const*>()
{
	read_skip<char*>();
}

template<>
inline void MemoryStream::read_skip<FString>()
{
	read_skip<char*>();
}

}