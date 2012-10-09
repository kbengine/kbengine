/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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


#ifndef __KBE_DEBUG_OPTION_HPP__
#define __KBE_DEBUG_OPTION_HPP__
#include "cstdkbe/cstdkbe.hpp"

namespace KBEngine{

namespace Mercury
{

/** 
	这个开关设置数据包是否总是携带长度信息， 这样在某些前端进行耦合时提供一些便利
	 如果为false则一些固定长度的数据包不携带长度信息， 由对端自行解析
*/
extern bool g_packetAlwaysContainLength;

/**
	是否需要将任何接收和发送的包以文本输出到log中提供调试
		0: 不输出
		1: 16进制输出
		2: 字符流输出
		3: 10进制输出
*/
extern uint8 g_trace_packet;

}

/**
	是否输出entity的创建， 脚本获取属性， 初始化属性等调试信息。
*/
extern bool g_debugEntity;

}

#endif // __KBE_DEBUG_OPTION_HPP__
