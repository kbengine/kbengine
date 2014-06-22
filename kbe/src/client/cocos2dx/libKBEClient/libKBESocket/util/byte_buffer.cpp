#include "byte_buffer.h"

NS_GC_BEGIN

ByteBuffer::ByteBuffer() :
_rpos(0), _wpos(0)
{
	_storage.reserve(DEFAULT_SIZE);
}
ByteBuffer::ByteBuffer(size_t res) :
_rpos(0), _wpos(0)
{
	_storage.reserve(res);
}
ByteBuffer::ByteBuffer(const ByteBuffer & buf) :
_rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage)
{
}
ByteBuffer::~ByteBuffer()
{
}
void ByteBuffer::clear()
{
	_storage.clear();
	_rpos = _wpos = 0;
}
ByteBuffer & ByteBuffer::operator<<(bool value)
{
	append<char> ((char) value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(uint8 value)
{
	append<uint8> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(uint16 value)
{
	append<uint16> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(uint32 value)
{
	append<uint32> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(uint64 value)
{
	append<uint64> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(int8 value)
{
	append<int8> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(int16 value)
{
	append<int16> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(int32 value)
{
	append<int32> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(int64 value)
{
	append<int64> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(float value)
{
	append<float> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(double value)
{
	append<double> (value);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(const std::string & value)
{
	append((uint8*) value.c_str(), value.length());
	append((uint8) 0);
	return *this;
}
ByteBuffer & ByteBuffer::operator<<(const char* str)
{
	append((uint8*) str, strlen(str));
	append((uint8) 0);
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(bool & value)
{
	value = read<char> () > 0 ? true : false;
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(uint8 & value)
{
	value = read<uint8> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(uint16 & value)
{
	value = read<uint16> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(uint32 & value)
{
	value = read<uint32> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(uint64 & value)
{
	value = read<uint64> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(int8 & value)
{
	value = read<int8> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(int16 & value)
{
	value = read<int16> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(int32 & value)
{
	value = read<int32> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(int64 & value)
{
	value = read<int64> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(float & value)
{
	value = read<float> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(double & value)
{
	value = read<double> ();
	return *this;
}
ByteBuffer & ByteBuffer::operator>>(std::string & value)
{
	value.clear();
	while (true)
	{
		char c = read<char> ();
		if (c == 0)
			break;
		value += c;
	}
	return *this;
}
uint8 ByteBuffer::operator[](size_t pos)
{
	return read<uint8> (pos);
}
size_t ByteBuffer::rpos()
{
	return _rpos;
}
size_t ByteBuffer::rpos(size_t rpos)
{
	_rpos = rpos;
	return _rpos;
}
size_t ByteBuffer::wpos()
{
	return _wpos;
}
size_t ByteBuffer::wpos(size_t wpos)
{
	_wpos = wpos;
	return _wpos;
}
void ByteBuffer::read(uint8* dest, size_t len)
{
	if (_rpos + len <= size())
	{
		memcpy(dest, &_storage[_rpos], len);
	}
	else
	{
		//throw error();
		memset(dest, 0, len);
	}
	_rpos += len;
}
const uint8* ByteBuffer::contents() const
{
	return &_storage[0];
}
size_t ByteBuffer::size() const
{
	return _storage.size();
}
size_t ByteBuffer::remaing() const
{
	return _wpos - _rpos;
}
void ByteBuffer::resize(size_t newsize)
{
	_storage.resize(newsize);
	_rpos = 0;
	_wpos = size();
}
void ByteBuffer::reserve(size_t ressize)
{
	if (ressize > size())
		_storage.reserve(ressize);
}
void ByteBuffer::append(const std::string & str)
{
	append((uint8*) str.c_str(), str.size() + 1);
}
void ByteBuffer::append(const char* src, size_t cnt)
{
	return append((const uint8*) src, cnt);
}
void ByteBuffer::append(const uint8* src, size_t cnt)
{
	if (!cnt)
		return;
		// noone should even need uint8buffer longer than 10mb
		// if you DO need, think about it
		// then think some more
		// then use something else
		// -- qz
		// ASSERT(size() < 10000000);


	// this way hopefully people will report the callstack after it "crashes"
	assert(size() < 10000000);

	if (_storage.size() < _wpos + cnt)
		_storage.resize(_wpos + cnt);
	memcpy(&_storage[_wpos], src, cnt);
	_wpos += cnt;
}
void ByteBuffer::append(const ByteBuffer & buffer)
{
	if (buffer.size() > 0)
		append(buffer.contents(), buffer.size());
}
void ByteBuffer::appendPackGUID(uint64 guid)
{
	size_t mask_position = wpos();
	*this << uint8(0);
	for (uint8 i = 0; i < 8; i++)
	{
		if (guid & 0xFF)
		{
			_storage[mask_position] |= (1 << i);
			*this << ((uint8)(guid & 0xFF));
		}
		guid >>= 8;
	}
}
uint64 ByteBuffer::unpackGUID()
{
	uint64 guid = 0;
	uint8 mask;
	uint8 temp;
	*this >> mask;
	for (uint8 i = 0; i < 8; ++i)
	{
		if (mask & (1 << i))
		{
			*this >> temp;
			guid |= uint64(temp << uint64(i << 3));
		}
	}
	return guid;
}
void ByteBuffer::put(size_t pos, const uint8* src, size_t cnt)
{
	assert(pos + cnt <= size());
	memcpy(&_storage[pos], src, cnt);
}
void ByteBuffer::hexlike()
{
	uint32 j = 1, k = 1;
	printf("STORAGE_SIZE: %u\n", (unsigned int) size());
	for (uint32 i = 0; i < size(); i++)
	{
		if ((i == (j * 8)) && ((i != (k * 16))))
		{
			if (read<uint8> (i) <= 0x0F)
			{
				printf(" 0%X ", read<uint8> (i));
			}
			else
			{
				printf(" %X ", read<uint8> (i));
			}

			j++;
		}
		else if (i == (k * 16))
		{
			rpos(rpos() - 16); // move read pointer 16 places back
			printf(" | "); // write split char

			for (int x = 0; x < 16; x++)
			{
				printf("%c", read<uint8> (i - 16 + x));
			}

			if (read<uint8> (i) <= 0x0F)
			{
				printf("\n0%X ", read<uint8> (i));
			}
			else
			{
				printf("\n%X ", read<uint8> (i));
			}

			k++;
			j++;
		}
		else
		{
			if (read<uint8> (i) <= 0x0F)
			{
				printf("0%X ", read<uint8> (i));
			}
			else
			{
				printf("%X ", read<uint8> (i));
			}
		}
	}
	printf("\n");
}
void ByteBuffer::reverse()
{
	std::reverse(_storage.begin(), _storage.end());
}

NS_GC_END