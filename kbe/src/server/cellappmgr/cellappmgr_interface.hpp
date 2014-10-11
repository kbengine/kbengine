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
	#undef KBE_CELLAPPMGR_INTERFACE_HPP
#endif


#ifndef KBE_CELLAPPMGR_INTERFACE_HPP
#define KBE_CELLAPPMGR_INTERFACE_HPP

// common include	
#if defined(CELLAPPMGR)
#include "cellappmgr.hpp"
#endif
#include "cellappmgr_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	BASEAPPMGR������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(CellappmgrInterface)
	// ĳappע���Լ��Ľӿڵ�ַ����app
	CELLAPPMGR_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,			MERCURY_VARIABLE_MESSAGE,
									int32,						uid, 
									std::string,				username,
									int8,						componentType, 
									uint64,						componentID, 
									int8,						globalorderID,
									int8,						grouporderID,
									uint32,						intaddr, 
									uint16,						intport,
									uint32,						extaddr, 
									uint16,						extport,
									std::string,				extaddrEx)

	// ĳapp��������look��
	CELLAPPMGR_MESSAGE_DECLARE_ARGS0(lookApp,					MERCURY_FIXED_MESSAGE)

	// ĳ��app����鿴��app����״̬��
	CELLAPPMGR_MESSAGE_DECLARE_ARGS0(queryLoad,					MERCURY_FIXED_MESSAGE)

	// ĳ��app��app��֪���ڻ״̬��
	CELLAPPMGR_MESSAGE_DECLARE_ARGS2(onAppActiveTick,			MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,				componentType, 
									COMPONENT_ID,				componentID)

	// baseEntity���󴴽���һ���µ�space�С�
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqCreateInNewSpace,		MERCURY_VARIABLE_MESSAGE)

	// baseEntity����ָ���һ���µ�space�С�
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqRestoreSpaceInCell,	MERCURY_VARIABLE_MESSAGE)

	// ��Ϣת���� ��ĳ��app��ͨ����app����Ϣת����ĳ��app��
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(forwardMessage,			MERCURY_VARIABLE_MESSAGE)

	// ����رշ�����
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqCloseServer,			MERCURY_VARIABLE_MESSAGE)

	// �����ѯwatcher����
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(queryWatcher,				MERCURY_VARIABLE_MESSAGE)

	// ����cellapp��Ϣ��
	CELLAPPMGR_MESSAGE_DECLARE_ARGS2(updateCellapp,				MERCURY_FIXED_MESSAGE,
									COMPONENT_ID,				componentID,
									float,						load)

	// ��ʼprofile
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(startProfile,				MERCURY_VARIABLE_MESSAGE)

	// ����ǿ��ɱ����ǰapp
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqKillServer,			MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
