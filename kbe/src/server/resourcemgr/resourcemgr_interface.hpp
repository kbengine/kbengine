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


#if defined(DEFINE_IN_INTERFACE)
	#undef __RESOURCEMGR_INTERFACE_H__
#endif


#ifndef __RESOURCEMGR_INTERFACE_H__
#define __RESOURCEMGR_INTERFACE_H__

// common include	
#if defined(RESOURCEMGR)
#include "resourcemgr.hpp"
#endif
#include "resourcemgr_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Resourcemgr������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(ResourcemgrInterface)
	// ĳappע���Լ��Ľӿڵ�ַ����app
	RESOURCEMGR_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,					MERCURY_VARIABLE_MESSAGE,
									int32,									uid, 
									std::string,							username,
									int8,									componentType, 
									uint64,									componentID, 
									int8,									globalorderID,
									int8,									grouporderID,
									uint32,									intaddr, 
									uint16,									intport,
									uint32,									extaddr, 
									uint16,									extport,
									std::string,							extaddrEx)

	// ĳ��app��app��֪���ڻ״̬��
	RESOURCEMGR_MESSAGE_DECLARE_ARGS2(onAppActiveTick,						MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID)

	// ĳapp��������look��
	RESOURCEMGR_MESSAGE_DECLARE_ARGS0(lookApp,								MERCURY_FIXED_MESSAGE)

	// ĳ��app����鿴��app����״̬��
	RESOURCEMGR_MESSAGE_DECLARE_ARGS0(queryLoad,							MERCURY_FIXED_MESSAGE)

	// ����رշ�����
	RESOURCEMGR_MESSAGE_DECLARE_STREAM(reqCloseServer,						MERCURY_VARIABLE_MESSAGE)

	// �����ѯwatcher����
	RESOURCEMGR_MESSAGE_DECLARE_STREAM(queryWatcher,						MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
