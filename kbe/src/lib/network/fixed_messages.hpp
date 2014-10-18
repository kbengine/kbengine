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

#ifndef KBE_FIXED_MERCURY_MESSAGES_HPP
#define KBE_FIXED_MERCURY_MESSAGES_HPP

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"

namespace KBEngine { 
namespace Mercury
{
/*
	用来对中间协议(前端与后端之间)进行强制约定。
	没有使用到kbe整套协议自动绑定机制的前端可以使用此处来强制约定协议。
*/
class FixedMessages : public Singleton<FixedMessages>
{
public:
	struct MSGInfo
	{
		MessageID msgid;
	};
public:
	FixedMessages();
	~FixedMessages();

	bool loadConfig(std::string fileName);

	FixedMessages::MSGInfo* isFixed(const char* msgName);
	bool isFixed(MessageID msgid);
public:
	typedef KBEUnordered_map<std::string, MSGInfo> MSGINFO_MAP;
private:
	MSGINFO_MAP _infomap;
	bool _loaded;
};

}
}
#endif // KBE_FIXED_MERCURY_MESSAGES_HPP
