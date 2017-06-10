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
	#undef KBE_BOTS_INTERFACE_H
#endif


#ifndef KBE_BOTS_INTERFACE_H
#define KBE_BOTS_INTERFACE_H

// common include	
#if defined(BOTS)
#include "bots.h"
#endif
#include "bots_interface_macros.h"
#include "network/interface_defs.h"
#include "client_lib/common.h"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Bots������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(BotsInterface)

	// ĳapp��������look��
	BOTS_MESSAGE_DECLARE_ARGS0(lookApp,									NETWORK_FIXED_MESSAGE)

	// ����رշ�����
	BOTS_MESSAGE_DECLARE_STREAM(reqCloseServer,							NETWORK_VARIABLE_MESSAGE)

	// consoleԶ��ִ��python��䡣
	BOTS_MESSAGE_DECLARE_STREAM(onExecScriptCommand,					NETWORK_VARIABLE_MESSAGE)

	// ĳ��app��app��֪���ڻ״̬��
	BOTS_MESSAGE_DECLARE_ARGS2(onAppActiveTick,							NETWORK_FIXED_MESSAGE,
								COMPONENT_TYPE,							componentType, 
								COMPONENT_ID,							componentID)

	// ���bots��
	BOTS_MESSAGE_DECLARE_STREAM(addBots,								NETWORK_VARIABLE_MESSAGE)

	// �����ѯwatcher����
	BOTS_MESSAGE_DECLARE_STREAM(queryWatcher,							NETWORK_VARIABLE_MESSAGE)

	// ��ʼprofile
	BOTS_MESSAGE_DECLARE_STREAM(startProfile,							NETWORK_VARIABLE_MESSAGE)

	// ����ǿ��ɱ����ǰapp
	BOTS_MESSAGE_DECLARE_STREAM(reqKillServer,							NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
