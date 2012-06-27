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

#ifndef __ENGINE_COMPONENT_MGR_H__
#define __ENGINE_COMPONENT_MGR_H__
	
// common include
//#define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "thread/threadmutex.hpp"
#include "thread/threadguard.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

namespace Mercury
{
class Channel;
}

class Components : public Singleton<Components>
{
public:
	struct ComponentInfos
	{
		uint32 addr;
		uint16 port;
		int32 uid;
		COMPONENT_ID cid;
		char username[256];
	};

	KBEngine::thread::ThreadMutex myMutex;
	typedef std::vector<ComponentInfos> COMPONENTS;

public:
	Components();
	~Components();

	uint32 allocComponentID(void);

	void addComponent(int32 uid, const char* username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, uint32 addr, uint16 port);

	void delComponent(int32 uid, COMPONENT_TYPE componentType, COMPONENT_ID componentID, bool ignoreComponentID = false);
	void clear(int32 uid);

	Components::COMPONENTS& getComponents(COMPONENT_TYPE componentType);

	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	//const Components::ComponentInfos findComponent(COMPONENT_TYPE componentType, int32 uid);
private:
	COMPONENTS			_baseapps;
	COMPONENTS			_cellapps;
	COMPONENTS			_dbmgrs;
	COMPONENTS			_loginapps;
	COMPONENTS			_cellappmgrs;
	COMPONENTS			_baseappmgrs;
	COMPONENTS			_machines;
	COMPONENTS			_centers;
};

}
#endif
