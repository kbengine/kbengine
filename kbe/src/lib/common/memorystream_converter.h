/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
	字节序交换处理模块：
		 由于网络通信一般采用BIG字节序\也叫做网络字节序.
 		 我们使用的PC机或者嵌入式系统可能使用BIG字节序也可能使用LITTEN(小字节序)
 		 所以我们必须在此之间做一个字节序的转换。
*/
#ifndef KBE_MEMORYSTREAMCONVERTER_H
#define KBE_MEMORYSTREAMCONVERTER_H

#include "common/common.h"
	
namespace KBEngine{

namespace MemoryStreamConverter
{
    template<size_t T>
        inline void convert(char *val)
    {
        std::swap(*val, *(val + T - 1));
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
		if(size < 2)
			return;

		std::swap(*val, *(val + size - 1));
		convert(val + 1, size - 2);
	}
}

#if KBENGINE_ENDIAN == KBENGINE_BIG_ENDIAN			// 可以使用sys.isPlatformLittleEndian() 进行测试
template<typename T> inline void EndianConvert(T& val) { MemoryStreamConverter::apply<T>(&val); }
template<typename T> inline void EndianConvertReverse(T&) { }
#else
template<typename T> inline void EndianConvert(T&) { }
template<typename T> inline void EndianConvertReverse(T& val) { MemoryStreamConverter::apply<T>(&val); }
#endif

template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error

inline void EndianConvert(uint8&) { }
inline void EndianConvert(int8&) { }
inline void EndianConvertReverse(uint8&) { }
inline void EndianConvertReverse(int8&) { }

}
#endif
