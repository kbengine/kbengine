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
class Address;
class NetworkInterface;
}

class Components : public Singleton<Components>
{
public:
	struct ComponentInfos
	{
		Mercury::Address* pIntAddr, *pExtAddr; // 内部和外部地址
		int32 uid;
		COMPONENT_ID cid;
		char username[MAX_NAME + 1];
		Mercury::Channel* pChannel;
	};

	KBEngine::thread::ThreadMutex myMutex;
	typedef std::vector<ComponentInfos> COMPONENTS;

public:
	Components();
	~Components();

	void pNetworkInterface(Mercury::NetworkInterface * networkInterface){ pNetworkInterface_ = networkInterface; }

	void addComponent(int32 uid, const char* username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
		uint32 intaddr, uint16 intport, 
		uint32 extaddr, uint16 extport, 
		Mercury::Channel* pChannel = NULL);

	void delComponent(int32 uid, COMPONENT_TYPE componentType, COMPONENT_ID componentID, bool ignoreComponentID = false);
	void clear(int32 uid);

	Components::COMPONENTS& getComponents(COMPONENT_TYPE componentType);

	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	//const Components::ComponentInfos findComponent(COMPONENT_TYPE componentType, int32 uid);

	int connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);
private:
	COMPONENTS					_baseapps;
	COMPONENTS					_cellapps;
	COMPONENTS					_dbmgrs;
	COMPONENTS					_loginapps;
	COMPONENTS					_cellappmgrs;
	COMPONENTS					_baseappmgrs;
	COMPONENTS					_machines;
	COMPONENTS					_centers;
	Mercury::NetworkInterface* 	pNetworkInterface_;
};

}
#endif
