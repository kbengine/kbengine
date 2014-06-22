//
//  KMemoryStream.h
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#ifndef __libKBEClient__KMemoryStream__
#define __libKBEClient__KMemoryStream__

#include "KBEClientcore.h"
#include "KMemoryStreamConvert.h"

//类型定义.

//#define KBEShared_ptr											std::tr1::shared_ptr
//#define KBEUnordered_map										std::tr1::unordered_map

typedef uint16													ENTITY_TYPE;											// entity的类别类型定义支持0-65535个类别
typedef int32													ENTITY_ID;												// entityID的类型
typedef uint32													SPACE_ID;												// 一个space的id
typedef uint32													CALLBACK_ID;											// 一个callback由CallbackMgr分配的id
typedef uint64													COMPONENT_ID;											// 一个服务器组件的id
typedef int8													COMPONENT_ORDER;										// 一个组件的启动顺序
typedef	uint32													TIMER_ID;												// 一个timer的id类型
typedef uint8													MAIL_TYPE;												// mailbox 所投递的mail类别的类别
typedef uint32													GAME_TIME;
typedef uint32													GameTime;
typedef int32													ScriptID;
typedef uint32													ArraySize;												// 任何数组的大小都用这个描述
typedef uint64													DBID;													// 一个在数据库中的索引用来当做某ID
typedef uint32													CELL_ID;
//typedef KBEUnordered_map< std::string, std::string >			SPACE_DATA;												// space中存储的数据


/** 定义服务器各组件类别 */
enum COMPONENT_TYPE
{
	UNKNOWN_COMPONENT_TYPE	= 0,
	DBMGR_TYPE				= 1,
	LOGINAPP_TYPE			= 2,
	BASEAPPMGR_TYPE			= 3,
	CELLAPPMGR_TYPE			= 4,
	CELLAPP_TYPE			= 5,
	BASEAPP_TYPE			= 6,
	CLIENT_TYPE				= 7,
	MACHINE_TYPE			= 8,
	CENTER_TYPE				= 9,
	CONSOLE_TYPE			= 10,
	MESSAGELOG_TYPE			= 11,
	RESOURCEMGR_TYPE		= 12,
	BOTS_TYPE				= 13,
	WATCHER_TYPE			= 14,
	BILLING_TYPE			= 15,
	COMPONENT_END_TYPE		= 16,
};
/** entity的mailbox类别 */
enum ENTITY_MAILBOX_TYPE
{
	MAILBOX_TYPE_CELL												= 0,
	MAILBOX_TYPE_BASE												= 1,
	MAILBOX_TYPE_CLIENT												= 2,
	MAILBOX_TYPE_CELL_VIA_BASE										= 3,
	MAILBOX_TYPE_BASE_VIA_CELL										= 4,
	MAILBOX_TYPE_CLIENT_VIA_CELL									= 5,
	MAILBOX_TYPE_CLIENT_VIA_BASE									= 6,
};

/** mailbox的类别对换为字符串名称 严格和ENTITY_MAILBOX_TYPE索引匹配 */
const char ENTITY_MAILBOX_TYPE_TO_NAME_TABLE[][8] = 
{
	"cell",
	"base",
	"client",
	"cell",
	"base",
	"client",
	"client",
};



/*---------------------------------------------------------------------------------
	跨平台接口定义
---------------------------------------------------------------------------------*/
#if defined( unix ) || defined( PLAYSTATION3 )

#define kbe_isnan isnan
#define kbe_isinf isinf
#define kbe_snprintf snprintf
#define kbe_vsnprintf vsnprintf
#define kbe_vsnwprintf vsnwprintf
#define kbe_snwprintf swprintf
#define kbe_stricmp strcasecmp
#define kbe_strnicmp strncasecmp
#define kbe_fileno fileno
#define kbe_va_copy va_copy
#else
#define kbe_isnan _isnan
#define kbe_isinf(x) (!_finite(x) && !_isnan(x))
#define kbe_snprintf _snprintf
#define kbe_vsnprintf _vsnprintf
#define kbe_vsnwprintf _vsnwprintf
#define kbe_snwprintf _snwprintf
#define kbe_stricmp _stricmp
#define kbe_strnicmp _strnicmp
#define kbe_fileno _fileno
#define kbe_va_copy( dst, src) dst = src

#define strtoq   _strtoi64
#define strtouq  _strtoui64
#define strtoll  _strtoi64
#define strtoull _strtoui64
#define atoll    _atoi64

#endif // unix

#define DEBUG_MSG(m)	cocos2d::CCLog(m)

namespace KBEngineClient{

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
class PoolObject {
};

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
			printf ("Attempted to %1% in MemoryStream (pos:%2%  size: %3%).\n" , 
				(_m_add ? "put" : "get") , _m_pos , _m_size);
        }
    private:
        bool 		_m_add;
        size_t 		_m_pos;
        size_t 		_m_esize;
        size_t 		_m_size;
};

class MemoryStream : public PoolObject
{
	union PackFloatXType
	{
		float	fv;
		uint32	uv;
		int		iv;
	};
public:
	//no ObjPool 
	//static ObjectPool<MemoryStream>& ObjPool();
	//static void destroyObjPool();

	//typedef KBEShared_ptr< SmartPoolObject< MemoryStream > > SmartPoolObjectPtr;
	//static SmartPoolObjectPtr createSmartPoolObj();

    const static size_t DEFAULT_SIZE = 0x1000;
    MemoryStream(): rpos_(0), wpos_(0)
    {
        data_.reserve(DEFAULT_SIZE);
    }

    MemoryStream(size_t res): rpos_(0), wpos_(0)
    {
		if(res <= 0)
			res = DEFAULT_SIZE;
        data_.reserve(res);
    }

    MemoryStream(const MemoryStream &buf): rpos_(buf.rpos_), wpos_(buf.wpos_), data_(buf.data_) { }
	
	virtual ~MemoryStream()
	{
		clear(true);
	}
	
	void onReclaimObject()
	{
		clear(false);
	}

    void clear(bool clearData)
    {
    	if(clearData)
      	  data_.clear();
        rpos_ = wpos_ = 0;
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
	
	void swap(MemoryStream & s)
	{
		size_t rpos = s.rpos(), wpos = s.wpos();
		std::swap(data_, s.data_);
		s.rpos(rpos_);
		s.wpos(wpos_);
		rpos_ = rpos;
		wpos_ = wpos;
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

    MemoryStream &operator<<(COMPONENT_TYPE value)
    {
        append<int32>(value);
        return *this;
    }

    MemoryStream &operator<<(ENTITY_MAILBOX_TYPE value)
    {
        append<int32>(value);
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

    MemoryStream &operator>>(std::string& value)
    {
        value.clear();
        while (opsize() > 0)
        {
            char c = read<char>();
            if (c == 0)
                break;

            value += c;
        }
        
        return *this;
    }

    MemoryStream &operator>>(char *value)
    {
        while (opsize() > 0)
        {
            char c = read<char>();
            if (c == 0)
                break;

            *(value++) = c;
        }

		*value = '\0';
        return *this;
    }
    
    MemoryStream &operator>>(COMPONENT_TYPE &value)
    {
        value = static_cast<COMPONENT_TYPE>(read<int32>());
        return *this;
    }

    MemoryStream &operator>>(ENTITY_MAILBOX_TYPE &value)
    {
        value = static_cast<ENTITY_MAILBOX_TYPE>(read<int32>());
        return *this;
    }

    uint8 operator[](size_t pos) const
    {
        return read<uint8>(pos);
    }

    size_t rpos() const { return rpos_; }

    size_t rpos(int rpos)
    {
		if(rpos < 0)
			rpos = 0;

        rpos_ = rpos;
        return rpos_;
    }

    size_t wpos() const { return wpos_; }

    size_t wpos(int wpos)
    {
		if(wpos < 0)
			wpos = 0;

        wpos_ = wpos;
        return wpos_;
    }

    template<typename T>
    void read_skip() { read_skip(sizeof(T)); }

    void read_skip(size_t skip)
    {
        if(skip > opsize())
            throw MemoryStreamException(false, rpos_, skip, opsize());
        rpos_ += skip;
    }

    template <typename T> T read()
    {
        T r = read<T>(rpos_);
        rpos_ += sizeof(T);
        return r;
    }

    template <typename T> T read(size_t pos) const
    {
        if(sizeof(T) > opsize())
            throw MemoryStreamException(false, pos, sizeof(T), opsize());
        T val = *((T const*)&data_[pos]);
        EndianConvert(val);
        return val;
    }

    void read(uint8 *dest, size_t len)
    {
        if(len > opsize())
           throw MemoryStreamException(false, rpos_, len, opsize());
        memcpy(dest, &data_[rpos_], len);
        rpos_ += len;
    }

	ArraySize readBlob(std::string& datas)
	{
		if(opsize() <= 0)
			return 0;

		ArraySize rsize = 0;
		(*this) >> rsize;
		if((size_t)rsize > opsize())
			return 0;

		if(rsize > 0)
		{
			datas.assign((char*)(data() + rpos()), rsize);
			read_skip(rsize);
		}

		return rsize;
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

    bool readPackGUID(uint64& guid)
    {
        if(opsize() == 0)
            return false;

        guid = 0;

        uint8 guidmark = 0;
        (*this) >> guidmark;

        for(int i = 0; i < 8; ++i)
        {
            if(guidmark & (uint8(1) << i))
            {
                if(opsize() == 0)
                    return false;

                uint8 bit;
                (*this) >> bit;
                guid |= (uint64(bit) << (i * 8));
            }
        }

        return true;
    }

    uint8 *data() { return &data_[0]; }
	const uint8 *data()const { return &data_[0]; }
		
    virtual size_t size() const { return data_.size(); }
    virtual bool empty() const { return data_.empty(); }
	size_t opsize()const { return rpos() >= wpos() ? 0 : wpos() - rpos(); }
	virtual size_t fillfree() const { return size() - wpos(); }

	void opfini(){ read_skip(opsize()); }

    void resize(size_t newsize)
    {
        data_.resize(newsize);
        rpos_ = 0;
        wpos_ = size();
    }

    void data_resize(size_t newsize)
    {
        data_.resize(newsize);
    }

    void reserve(size_t ressize)
    {
        if (ressize > size())
            data_.reserve(ressize);
    }

    void appendBlob(const char *src, ArraySize cnt)
    {
        (*this) << cnt;
		if(cnt > 0)
			append(src, cnt);
    }

	void appendBlob(const std::string& datas)
    {
		ArraySize len = datas.size();
		(*this) << len;
		if(len > 0)
			append(datas.data(), len);
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

        if (data_.size() < wpos_ + cnt)
            data_.resize(wpos_ + cnt);
        memcpy(&data_[wpos_], src, cnt);
        wpos_ += cnt;
    }

    void append(const MemoryStream& buffer)
    {
        if(buffer.wpos()){
			append(buffer.data() + buffer.rpos(), buffer.opsize());
        }
    }

    void appendPackAnyXYZ(float x, float y, float z, const float epsilon = 0.5f)
    {
		if(epsilon > 0.f)
		{
			x = floorf(x + epsilon);
			y = floorf(y + epsilon);
			z = floorf(z + epsilon);
		}

        *this << x << y << z;
    }

    void appendPackAnyXZ(float x, float z, const float epsilon = 0.5f)
    {
		if(epsilon > 0.f)
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
		data |= ((xPackData.uv >>  3) & 0x7ff000) + ((xPackData.uv & 0x4000) >> 2);
		data |= ((zPackData.uv >> 15) & 0x0007ff) + ((zPackData.uv & 0x4000) >> 14);
		
		// 确保值在范围内
		// 0x7ff7ff = 11111111111011111111111
		data &= 0x7ff7ff;

		// 复制标记位
		// 0x800000 = 100000000000000000000000
		// 0x000800 = 000000000000100000000000
		data |=  (xPackData.uv >>  8) & 0x800000;
		data |=  (zPackData.uv >> 20) & 0x000800;

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

    void appendPackGUID(uint64 guid)
    {
        if (data_.size() < wpos_ + sizeof(guid) + 1)
            data_.resize(wpos_ + sizeof(guid) + 1);

        size_t mask_position = wpos();
        *this << uint8(0);
        for(uint8 i = 0; i < 8; ++i)
        {
            if(guid & 0xFF)
            {
                data_[mask_position] |= uint8(1 << i);
                *this << uint8(guid & 0xFF);
            }

            guid >>= 8;
        }
    }

    void put(size_t pos, const uint8 *src, size_t cnt)
    {
        if(pos + cnt > size())
           throw MemoryStreamException(true, pos, cnt, size());
        memcpy(&data_[pos], src, cnt);
    }

	/** 输出流数据 */
    void print_storage() const
    {
		char buf[1024];
		std::string fbuffer;
		size_t trpos = rpos_;

		kbe_snprintf(buf, 1024, "STORAGE_SIZE: %lu, rpos=%lu.\n", (unsigned long)wpos(), (unsigned long)rpos());
		fbuffer += buf;

        for(uint32 i = rpos(); i < wpos(); ++i)
		{
			kbe_snprintf(buf, 1024, "%u ", read<uint8>(i));
			fbuffer += buf;
		}

		fbuffer += " \n";
        DEBUG_MSG(fbuffer.c_str());

		rpos_ = trpos;
    }

	/** 输出流数据字符串 */
    void textlike() const
    {
		char buf[1024];
		std::string fbuffer;
		size_t trpos = rpos_;

		kbe_snprintf(buf, 1024, "STORAGE_SIZE: %lu, rpos=%lu.\n", (unsigned long)wpos(), (unsigned long)rpos());
		fbuffer += buf;

        for(uint32 i = rpos(); i < wpos(); ++i)
		{
			kbe_snprintf(buf, 1024, "%c", read<uint8>(i));
			fbuffer += buf;
		}

		fbuffer += " \n";
        DEBUG_MSG(fbuffer.c_str());

		rpos_ = trpos;
    }

    void hexlike() const
    {
        uint32 j = 1, k = 1;
		char buf[1024];
		std::string fbuffer;
		size_t trpos = rpos_;

		kbe_snprintf(buf, 1024, "STORAGE_SIZE: %lu, rpos=%lu.\n", (unsigned long)wpos(), (unsigned long)rpos());
		fbuffer += buf;
		
		uint32 i = 0;
        for(uint32 idx = rpos(); idx < wpos(); ++idx)
        {
			++i;
            if ((i == (j * 8)) && ((i != (k * 16))))
            {
                if (read<uint8>(idx) < 0x10)
                {
					kbe_snprintf(buf, 1024, "| 0%X ", read<uint8>(idx));
					fbuffer += buf;
                }
                else
                {
					kbe_snprintf(buf, 1024, "| %X ", read<uint8>(idx));
					fbuffer += buf;
                }
                ++j;
            }
            else if (i == (k * 16))
            {
                if (read<uint8>(idx) < 0x10)
                {
					kbe_snprintf(buf, 1024, "\n0%X ", read<uint8>(idx));
					fbuffer += buf;
                }
                else
                {
					kbe_snprintf(buf, 1024, "\n%X ", read<uint8>(idx));
					fbuffer += buf;
                }

                ++k;
                ++j;
            }
            else
            {
                if (read<uint8>(idx) < 0x10)
                {
					kbe_snprintf(buf, 1024, "0%X ", read<uint8>(idx));
					fbuffer += buf;
                }
                else
                {
					kbe_snprintf(buf, 1024, "%X ", read<uint8>(idx));
					fbuffer += buf;
                }
            }
        }

		fbuffer += "\n";

		DEBUG_MSG(fbuffer.c_str());

		rpos_ = trpos;
    }
protected:
	mutable size_t rpos_, wpos_;
	std::vector<uint8> data_;
};


template <typename T>
inline MemoryStream &operator<<(MemoryStream &b, std::vector<T> v)
{
	uint32 vsize = v.size();
    b << vsize;
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline MemoryStream &operator>>(MemoryStream &b, std::vector<T> &v)
{
    ArraySize vsize;
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
	ArraySize vsize = v.size();
    b << vsize;
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline MemoryStream &operator>>(MemoryStream &b, std::list<T> &v)
{
    ArraySize vsize;
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
	ArraySize vsize = m.size();
    b << vsize;
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V>
inline MemoryStream &operator>>(MemoryStream &b, std::map<K, V> &m)
{
    ArraySize msize;
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

// 从对象池中创建与回收
#define NEW_MEMORY_STREAM() MemoryStream::ObjPool().createObject()
#define DELETE_MEMORY_STREAM(obj) { MemoryStream::ObjPool().reclaimObject(obj); obj = NULL; }

}
#endif /* defined(__libKBEClient__KMemoryStream__) */
