/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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
	#undef KBE_LOGGER_INTERFACE_H
#endif


#ifndef KBE_LOGGER_INTERFACE_H
#define KBE_LOGGER_INTERFACE_H

// common include	
#if defined(LOGGER)
#include "logger.h"
#endif
#include "logger_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Logger������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(LoggerInterface)
	// ĳappע���Լ��Ľӿڵ�ַ����app
	LOGGER_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,							NETWORK_VARIABLE_MESSAGE,
									int32,									uid, 
									std::string,							username,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID, 
									COMPONENT_ORDER,						globalorderID,
									COMPONENT_ORDER,						grouporderID,
									uint32,									intaddr, 
									uint16,									intport,
									uint32,									extaddr, 
									uint16,									extport,
									std::string,							extAddrEx)

	// ĳapp��������look��
	LOGGER_MESSAGE_DECLARE_ARGS0(lookApp,									NETWORK_FIXED_MESSAGE)

	// ĳ��app����鿴��app����״̬��
	LOGGER_MESSAGE_DECLARE_ARGS0(queryLoad,									NETWORK_FIXED_MESSAGE)

	// ĳ��app��app��֪���ڻ״̬��
	LOGGER_MESSAGE_DECLARE_ARGS2(onAppActiveTick,							NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID)

	// Զ��д��־
	LOGGER_MESSAGE_DECLARE_STREAM(writeLog,									NETWORK_VARIABLE_MESSAGE)

	// ע��log������
	LOGGER_MESSAGE_DECLARE_STREAM(registerLogWatcher,						NETWORK_VARIABLE_MESSAGE)

	// ע��log������
	LOGGER_MESSAGE_DECLARE_STREAM(deregisterLogWatcher,						NETWORK_VARIABLE_MESSAGE)

	// log�����߸����Լ�������
	LOGGER_MESSAGE_DECLARE_STREAM(updateLogWatcherSetting,					NETWORK_VARIABLE_MESSAGE)

	// ����رշ�����
	LOGGER_MESSAGE_DECLARE_STREAM(reqCloseServer,							NETWORK_VARIABLE_MESSAGE)

	// �����ѯwatcher����
	LOGGER_MESSAGE_DECLARE_STREAM(queryWatcher,								NETWORK_VARIABLE_MESSAGE)

	// ��ʼprofile
	LOGGER_MESSAGE_DECLARE_STREAM(startProfile,								NETWORK_VARIABLE_MESSAGE)

	// ����ǿ��ɱ����ǰapp
	LOGGER_MESSAGE_DECLARE_STREAM(reqKillServer,							NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
