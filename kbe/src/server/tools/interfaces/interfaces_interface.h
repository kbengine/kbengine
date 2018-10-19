// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_INTERFACES_TOOL_INTERFACE_H
#endif


#ifndef KBE_INTERFACES_TOOL_INTERFACE_H
#define KBE_INTERFACES_TOOL_INTERFACE_H

// common include	
#if defined(INTERFACES)
#include "interfaces.h"
#endif
#include "interfaces_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Interfaces��Ϣ�꣬  ����Ϊ���� ��Ҫ�Լ��⿪
*/

/**
	Interfaces������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(InterfacesInterface)
	// ĳappע���Լ��Ľӿڵ�ַ����app
	INTERFACES_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,						NETWORK_VARIABLE_MESSAGE,
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
									std::string,							extaddrEx)

	// ���󴴽��˺š�
	INTERFACES_MESSAGE_DECLARE_STREAM(reqCreateAccount,						NETWORK_VARIABLE_MESSAGE)

	// ��½�˺š�
	INTERFACES_MESSAGE_DECLARE_STREAM(onAccountLogin,						NETWORK_VARIABLE_MESSAGE)

	// ��ֵ����
	INTERFACES_MESSAGE_DECLARE_STREAM(charge,								NETWORK_VARIABLE_MESSAGE)

	// ĳapp��������look��
	INTERFACES_MESSAGE_DECLARE_ARGS0(lookApp,								NETWORK_FIXED_MESSAGE)

	// ĳ��app��app��֪���ڻ״̬��
	INTERFACES_MESSAGE_DECLARE_ARGS2(onAppActiveTick,						NETWORK_FIXED_MESSAGE,
										COMPONENT_TYPE,						componentType, 
										COMPONENT_ID,						componentID)

	// ����رշ�����
	INTERFACES_MESSAGE_DECLARE_STREAM(reqCloseServer,						NETWORK_VARIABLE_MESSAGE)

	// �����ѯwatcher����
	INTERFACES_MESSAGE_DECLARE_STREAM(queryWatcher,							NETWORK_VARIABLE_MESSAGE)

	// ��������ͻ�����������
	INTERFACES_MESSAGE_DECLARE_ARGS1(eraseClientReq,						NETWORK_VARIABLE_MESSAGE,
										std::string,						logkey)

	// ��ʼprofile
	INTERFACES_MESSAGE_DECLARE_STREAM(startProfile,							NETWORK_VARIABLE_MESSAGE)

	// ����ǿ��ɱ����ǰapp
	INTERFACES_MESSAGE_DECLARE_STREAM(reqKillServer,						NETWORK_VARIABLE_MESSAGE)

	// executeRawDatabaseCommand��dbmgr�Ļص�
	INTERFACES_MESSAGE_DECLARE_STREAM(onExecuteRawDatabaseCommandCB,		NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}

#endif // KBE_INTERFACES_TOOL_INTERFACE_H
