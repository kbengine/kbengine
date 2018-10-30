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

#ifndef KBE_SSL_H
#define KBE_SSL_H

#include "common/common.h"

namespace KBEngine
{

class MemoryStream;

class KB_SSL
{
public:
	static bool initialize();
	static void finalise();

	/**
		是否为Https/Wss协议
		返回具体协议版本
	*/
	static int isSSLProtocal(MemoryStream* s);

};

}

#endif // KBE_SSL_H
