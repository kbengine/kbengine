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

#ifndef __FORWARD_MESSAGEBUFFER__
#define __FORWARD_MESSAGEBUFFER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/tasks.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "server/components.hpp"

namespace KBEngine { 
namespace Mercury
{
class Bundle;
class NetworkInterface;
class EventDispatcher;
}

/*
	如果在app上没有找到任何cellapp或者baseapp这个模块将一些消息缓存起来， 
	等待有新的cellapp或者baseapp加入则开始将指令转发。
*/
class Forward_MessageBuffer : public Task, 
						public Singleton<Forward_MessageBuffer>
{
public:
	Forward_MessageBuffer(Mercury::NetworkInterface & networkInterface, COMPONENT_TYPE forwardComponentType);
	~Forward_MessageBuffer();

	Mercury:: EventDispatcher & dispatcher();

	void push(Mercury::Bundle* pBundle);
	
	bool process();
private:
	Mercury::NetworkInterface & networkInterface_;
	COMPONENT_TYPE forwardComponentType_;
	bool start_;
	
	std::vector<Mercury::Bundle*> pBundles_;

};

}

#endif // __FORWARD_MESSAGEBUFFER__
