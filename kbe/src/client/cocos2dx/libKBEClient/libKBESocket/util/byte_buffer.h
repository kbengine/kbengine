#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <list>
#include <vector>
#include <map>
#include <string>
#include <assert.h>
#include <algorithm>
#include "basic_types.h"

NS_GC_BEGIN

class ByteBuffer
{
public:
	const static size_t DEFAULT_SIZE = 0x1000;

	ByteBuffer();
	ByteBuffer(size_t res);
	ByteBuffer(const ByteBuffer & buf);
	virtual ~ByteBuffer();

	void clear();

	//template <typename T> void insert(size_t pos, T value) {
	//  insert(pos, (uint8 *)&value, sizeof(value));
	//}
	template<typename T> inline void append(T value)
	{
		append((uint8*) &value, sizeof(value));
	}
	template<typename T> inline void put(size_t pos, T value)
	{
		put(pos, (uint8*) &value, sizeof(value));
	}
	template<typename T> inline T read()
	{
		T r = read<T> (_rpos);
		_rpos += sizeof(T);
		return r;
	}
	template<typename T> inline T read(size_t pos) const
	{
		if (pos + sizeof(T) > size())
		{
			return (T) 0;
		}
		else
		{
			return *((T*) &_storage[pos]);
		}
	}

	// stream like operators for storing data
	ByteBuffer & operator<<(bool value);
	// unsigned
	ByteBuffer & operator<<(uint8 value);
	ByteBuffer & operator<<(uint16 value);
	ByteBuffer & operator<<(uint32 value);
	ByteBuffer & operator<<(uint64 value);
	// signed as in 2e complement
	ByteBuffer & operator<<(int8 value);
	ByteBuffer & operator<<(int16 value);
	ByteBuffer & operator<<(int32 value);
	ByteBuffer & operator<<(int64 value);
	ByteBuffer & operator<<(float value);
	ByteBuffer & operator<<(double value);
	ByteBuffer & operator<<(const std::string & value);
	ByteBuffer & operator<<(const char* str);
	// stream like operators for reading data
	ByteBuffer & operator>>(bool & value);
	//unsigned
	ByteBuffer & operator>>(uint8 & value);
	ByteBuffer & operator>>(uint16 & value);
	ByteBuffer & operator>>(uint32 & value);
	ByteBuffer & operator>>(uint64 & value);
	//signed as in 2e complement
	ByteBuffer & operator>>(int8 & value);
	ByteBuffer & operator>>(int16 & value);
	ByteBuffer & operator>>(int32 & value);
	ByteBuffer & operator>>(int64 & value);
	ByteBuffer & operator>>(float & value);
	ByteBuffer & operator>>(double & value);
	ByteBuffer & operator>>(std::string & value);
	uint8 operator[](size_t pos);

	size_t rpos();
	size_t rpos(size_t rpos);
	size_t wpos();
	size_t wpos(size_t wpos);

	void read(uint8* dest, size_t len);
	const uint8* contents() const;
	size_t size() const;
	size_t remaing() const;
	// one should never use resize probably
	void resize(size_t newsize);
	void reserve(size_t ressize);
	// appending to the end of buffer
	void append(const std::string & str);
	void append(const char* src, size_t cnt);
	void append(const uint8* src, size_t cnt);
	void append(const ByteBuffer & buffer);
	void appendPackGUID(uint64 guid);
	uint64 unpackGUID();

	void put(size_t pos, const uint8* src, size_t cnt);
	//void insert(size_t pos, const uint8 *src, size_t cnt) {
	//  std::copy(src, src + cnt, inserter(_storage, _storage.begin() + pos));
	//}

	void hexlike();
	void reverse();

protected:
	// read and write positions
	size_t _rpos, _wpos;
	std::vector<uint8> _storage;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T> inline ByteBuffer & operator<<(ByteBuffer & b, std::vector<T> v)
{
	b << (uint32) v.size();
	for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++)
	{
		b << *i;
	}
	return b;
}
template<typename T> inline ByteBuffer & operator>>(ByteBuffer & b, std::vector<T> &v)
{
	uint32 vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}
template<typename T> inline ByteBuffer & operator<<(ByteBuffer & b, std::list<T> v)
{
	b << (uint32) v.size();
	for (typename std::list<T>::iterator i = v.begin(); i != v.end(); i++)
	{
		b << *i;
	}
	return b;
}
template<typename T> inline ByteBuffer & operator>>(ByteBuffer & b, std::list<T> &v)
{
	uint32 vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}
template<typename K, typename V> inline ByteBuffer & operator<<(ByteBuffer & b,
	std::map<K, V> &m)
{
	b << (uint32) m.size();
	for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); i++)
	{
		b << i->first << i->second;
	}
	return b;
}
template<typename K, typename V> inline ByteBuffer & operator>>(ByteBuffer & b,
	std::map<K, V> &m)
{
	uint32 msize;
	b >> msize;
	m.clear();
	while (msize--)
	{
		K k;
		V v;
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}

NS_GC_END

#endif