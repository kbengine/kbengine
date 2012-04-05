/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
/*
	byte流：
		能够将一些常用的数据类型以byte数据存储在一个vector中， 该对于一些占位比较大的数据类型
		再最后一个位置存放一个0标示结束， 这种存储结构适合网络间打包传输一些数据。
		
		注意：网络间传输可能涉及到一些字节序的问题， 需要进行转换。
		具体看 MemoryStreamConverter.hpp

	使用方法:
			MemoryStream stream; 
			stream << (int64)100000000;
			stream << (uint8)1;
			stream << (uint8)32;
			stream << "kebiao";
			stream.print_storage();
			uint8 n, n1;
			int64 x;
			std::string a;
			stream >> x;
			stream >> n;
			stream >> n1;
			stream >> a;
			printf("还原: %lld, %d, %d, %s", x, n, n1, a.c_str());
*/
#ifndef __BYTESTREAM_H__
#define __BYTESTREAM_H__
// common include	
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <assert.h>
#include "cstdkbe/cstdkbe.hpp"
#include "log/debug_helper.hpp"
#include "memorystream_converter.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#include <windows.h>
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

class MemoryStreamException
{
    public:
        MemoryStreamException(bool _add, size_t _pos, size_t _esize, size_t _size)
            : _m_add(_add), _m_pos(_pos), _m_esize(_esize), _m_size(_size)
        {
            PrintPosError();
        }

        void PrintPosError() const
        {
            ERROR_MSG("Attempted to %s in MemoryStream (pos:%d  size: %d).\n" , (_m_add ? "put" : "get"), _m_pos, _m_size);
        }
    private:
        bool 		_m_add;
        size_t 		_m_pos;
        size_t 		_m_esize;
        size_t 		_m_size;
};

class MemoryStream
{
    public:
        const static size_t DEFAULT_SIZE = 0x1000;
        MemoryStream(): m_rpos_(0), m_wpos_(0)
        {
            m_storage_.reserve(DEFAULT_SIZE);
        }

        MemoryStream(size_t res): m_rpos_(0), m_wpos_(0)
        {
            m_storage_.reserve(res);
        }

        MemoryStream(const MemoryStream &buf): m_rpos_(buf.m_rpos_), m_wpos_(buf.m_wpos_), m_storage_(buf.m_storage_) { }

        void clear()
        {
            m_storage_.clear();
            m_rpos_ = m_wpos_ = 0;
        }

        template <typename T> void append(T value)
        {
            EndianConvert(value);
            append((uint8 *)&value, sizeof(value));
        }

        template <typename T> void put(size_t pos,T value)
        {
            EndianConvert(value);
            put(pos,(uint8 *)&value,sizeof(value));
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

        MemoryStream &operator<<(const std::string &value)
        {
            append((uint8 const *)value.c_str(), value.length());
            append((uint8)0);
            return *this;
        }

        MemoryStream &operator<<(const char *str)
        {
            append((uint8 const *)str, str ? strlen(str) : 0);
            append((uint8)0);
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

        MemoryStream &operator>>(std::string& value)
        {
            value.clear();
            while (rpos() < size())
            {
                char c = read<char>();
                if (c == 0)
                    break;
                value += c;
            }
            return *this;
        }

        uint8 operator[](size_t pos) const
        {
            return read<uint8>(pos);
        }

        size_t rpos() const { return m_rpos_; }

        size_t rpos(size_t rpos_)
        {
            m_rpos_ = rpos_;
            return m_rpos_;
        }

        size_t wpos() const { return m_wpos_; }

        size_t wpos(size_t wpos_)
        {
            m_wpos_ = wpos_;
            return m_wpos_;
        }

        template<typename T>
        void read_skip() { read_skip(sizeof(T)); }

        void read_skip(size_t skip)
        {
            if(m_rpos_ + skip > size())
                throw MemoryStreamException(false, m_rpos_, skip, size());
            m_rpos_ += skip;
        }

        template <typename T> T read()
        {
            T r = read<T>(m_rpos_);
            m_rpos_ += sizeof(T);
            return r;
        }

        template <typename T> T read(size_t pos) const
        {
            if(pos + sizeof(T) > size())
                throw MemoryStreamException(false, pos, sizeof(T), size());
            T val = *((T const*)&m_storage_[pos]);
            EndianConvert(val);
            return val;
        }

        void read(uint8 *dest, size_t len)
        {
            if(m_rpos_  + len > size())
               throw MemoryStreamException(false, m_rpos_, len, size());
            memcpy(dest, &m_storage_[m_rpos_], len);
            m_rpos_ += len;
        }

        bool readPackGUID(uint64& guid)
        {
            if(rpos() + 1 > size())
                return false;

            guid = 0;

            uint8 guidmark = 0;
            (*this) >> guidmark;

            for(int i = 0; i < 8; ++i)
            {
                if(guidmark & (uint8(1) << i))
                {
                    if(rpos() + 1 > size())
                        return false;

                    uint8 bit;
                    (*this) >> bit;
                    guid |= (uint64(bit) << (i * 8));
                }
            }

            return true;
        }

        const uint8 *contents() const { return &m_storage_[0]; }

        size_t size() const { return m_storage_.size(); }
        bool empty() const { return m_storage_.empty(); }

        void resize(size_t newsize)
        {
            m_storage_.resize(newsize);
            m_rpos_ = 0;
            m_wpos_ = size();
        }

        void reserve(size_t ressize)
        {
            if (ressize > size())
                m_storage_.reserve(ressize);
        }

        void append(const std::string& str)
        {
            append((uint8 const*)str.c_str(), str.size() + 1);
        }

        void append(const char *src, size_t cnt)
        {
            return append((const uint8 *)src, cnt);
        }

        template<class T> void append(const T *src, size_t cnt)
        {
            return append((const uint8 *)src, cnt * sizeof(T));
        }

        void append(const uint8 *src, size_t cnt)
        {
            if (!cnt)
                return;

            assert(size() < 10000000);

            if (m_storage_.size() < m_wpos_ + cnt)
                m_storage_.resize(m_wpos_ + cnt);
            memcpy(&m_storage_[m_wpos_], src, cnt);
            m_wpos_ += cnt;
        }

        void append(const MemoryStream& buffer)
        {
            if(buffer.wpos())
                append(buffer.contents(), buffer.wpos());
        }

        /** 将一个坐标信息加入到流 */
        void appendPackXYZ(float x, float y, float z)
        {
            uint32 packed = 0;
            packed |= ((int)(x / 0.25f) & 0x7FF);
            packed |= ((int)(y / 0.25f) & 0x7FF) << 11;
            packed |= ((int)(z / 0.25f) & 0x3FF) << 22;
            *this << packed;
        }

        void appendPackGUID(uint64 guid)
        {
            if (m_storage_.size() < m_wpos_ + sizeof(guid) + 1)
                m_storage_.resize(m_wpos_ + sizeof(guid) + 1);

            size_t mask_position = wpos();
            *this << uint8(0);
            for(uint8 i = 0; i < 8; ++i)
            {
                if(guid & 0xFF)
                {
                    m_storage_[mask_position] |= uint8(1 << i);
                    *this << uint8(guid & 0xFF);
                }

                guid >>= 8;
            }
        }

        void put(size_t pos, const uint8 *src, size_t cnt)
        {
            if(pos + cnt > size())
               throw MemoryStreamException(true, pos, cnt, size());
            memcpy(&m_storage_[pos], src, cnt);
        }

		/** 输出流数据 */
        void print_storage() const
        {
            DEBUG_MSG("STORAGE_SIZE: %lu\n", (unsigned long)size());
            for(uint32 i = 0; i < size(); ++i)
                PRINT_MSG("%u - \n", read<uint8>(i));
            PRINT_MSG(" ");
        }

		/** 输出流数据字符串 */
        void textlike() const
        {
            DEBUG_MSG("STORAGE_SIZE: %lu\n", (unsigned long)size());
            for(uint32 i = 0; i < size(); ++i)
                PRINT_MSG("%c", read<uint8>(i));
            PRINT_MSG(" ");
        }

        void hexlike() const
        {
            uint32 j = 1, k = 1;
            DEBUG_MSG("STORAGE_SIZE: %lu\n", (unsigned long)size());

            for(uint32 i = 0; i < size(); ++i)
            {
                if ((i == (j * 8)) && ((i != (k * 16))))
                {
                    if (read<uint8>(i) < 0x10)
                    {
                        PRINT_MSG("| 0%X ", read<uint8>(i));
                    }
                    else
                    {
                        PRINT_MSG("| %X ", read<uint8>(i));
                    }
                    ++j;
                }
                else if (i == (k * 16))
                {
                    if (read<uint8>(i) < 0x10)
                    {
                        PRINT_MSG("\n");
                        PRINT_MSG("0%X ", read<uint8>(i));
                    }
                    else
                    {
                        PRINT_MSG("\n");
                        PRINT_MSG("%X ", read<uint8>(i));
                    }

                    ++k;
                    ++j;
                }
                else
                {
                    if (read<uint8>(i) < 0x10)
                    {
                        PRINT_MSG("0%X ", read<uint8>(i));
                    }
                    else
                    {
                        PRINT_MSG("%X ", read<uint8>(i));
                    }
                }
            }
            PRINT_MSG("\n");
        }
    protected:
        size_t m_rpos_, m_wpos_;
        std::vector<uint8> m_storage_;
};

template <typename T>
inline MemoryStream &operator<<(MemoryStream &b, std::vector<T> v)
{
    b << (uint32)v.size();
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline MemoryStream &operator>>(MemoryStream &b, std::vector<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename T>
inline MemoryStream &operator<<(MemoryStream &b, std::list<T> v)
{
    b << (uint32)v.size();
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline MemoryStream &operator>>(MemoryStream &b, std::list<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename K, typename V>
inline MemoryStream &operator<<(MemoryStream &b, std::map<K, V> &m)
{
    b << (uint32)m.size();
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V>
inline MemoryStream &operator>>(MemoryStream &b, std::map<K, V> &m)
{
    uint32 msize;
    b >> msize;
    m.clear();
    while(msize--)
    {
        K k;
        V v;
        b >> k >> v;
        m.insert(make_pair(k, v));
    }
    return b;
}

template<>
inline void MemoryStream::read_skip<char*>()
{
    std::string temp;
    *this >> temp;
}

template<>
inline void MemoryStream::read_skip<char const*>()
{
    read_skip<char*>();
}

template<>
inline void MemoryStream::read_skip<std::string>()
{
    read_skip<char*>();
}

}
#endif
