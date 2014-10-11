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
	#undef KBE_DBMGR_INTERFACE_HPP
#endif


#ifndef KBE_DBMGR_INTERFACE_HPP
#define KBE_DBMGR_INTERFACE_HPP

// common include	
#if defined(DBMGR)
#include "dbmgr.hpp"
#endif
#include "dbmgr_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Dbmgr��Ϣ�꣬  ����Ϊ���� ��Ҫ�Լ��⿪
*/

/**
	DBMGR������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(DbmgrInterface)
	// ĳappע���Լ��Ľӿڵ�ַ����app
	DBMGR_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,					MERCURY_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									int8,							componentType, 
									uint64,							componentID, 
									int8,							globalorderID,
									int8,							grouporderID,
									uint32,							intaddr, 
									uint16,							intport,
									uint32,							extaddr, 
									uint16,							extport,
									std::string,					extaddrEx)

	// ĳapp��������look��
	DBMGR_MESSAGE_DECLARE_ARGS0(lookApp,							MERCURY_FIXED_MESSAGE)

	// ĳ��app����鿴��app����״̬��
	DBMGR_MESSAGE_DECLARE_ARGS0(queryLoad,							MERCURY_FIXED_MESSAGE)

	// ĳapp�����ȡһ��entityID�� 
	DBMGR_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,					MERCURY_FIXED_MESSAGE,
								int8,								componentType,
								COMPONENT_ID,						componentID)

	// global���ݸı�
	DBMGR_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChanged,		MERCURY_VARIABLE_MESSAGE)

	// ĳ��app��app��֪���ڻ״̬��
	DBMGR_MESSAGE_DECLARE_ARGS2(onAppActiveTick,					MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID)

	// loginapp���󴴽��˺š�
	DBMGR_MESSAGE_DECLARE_STREAM(reqCreateAccount,					MERCURY_VARIABLE_MESSAGE)
	DBMGR_MESSAGE_DECLARE_STREAM(onCreateAccountCBFromBilling,		MERCURY_VARIABLE_MESSAGE)

	// ��½�˺š�
	DBMGR_MESSAGE_DECLARE_STREAM(onAccountLogin,					MERCURY_VARIABLE_MESSAGE)
	DBMGR_MESSAGE_DECLARE_STREAM(onLoginAccountCBBFromBilling,		MERCURY_VARIABLE_MESSAGE)

	// baseapp��ѯ�˺���Ϣ��
	DBMGR_MESSAGE_DECLARE_ARGS7(queryAccount,						MERCURY_VARIABLE_MESSAGE,
									std::string,					accountName,
									std::string,					password,
									COMPONENT_ID,					componentID,
									ENTITY_ID,						entityID,
									DBID,							entityDBID,
									uint32,							ip,
									uint16,							port)

	// baseapp���˺����ߡ�
	DBMGR_MESSAGE_DECLARE_ARGS3(onAccountOnline,					MERCURY_VARIABLE_MESSAGE,
									std::string,					accountName,
									COMPONENT_ID,					componentID,
									ENTITY_ID,						entityID)
		
	// baseapp��entity���ߡ�
	DBMGR_MESSAGE_DECLARE_ARGS2(onEntityOffline,					MERCURY_VARIABLE_MESSAGE,
									DBID,							dbid,
									uint16,							sid)

	// ��������ͻ�����������
	DBMGR_MESSAGE_DECLARE_ARGS1(eraseClientReq,						MERCURY_VARIABLE_MESSAGE,
									std::string,					logkey)

	// ���ݿ��ѯ
	DBMGR_MESSAGE_DECLARE_STREAM(executeRawDatabaseCommand,			MERCURY_VARIABLE_MESSAGE)

	// ĳ��entity�浵
	DBMGR_MESSAGE_DECLARE_STREAM(writeEntity,						MERCURY_VARIABLE_MESSAGE)

	// ɾ��ĳ��entity�Ĵ浵
	DBMGR_MESSAGE_DECLARE_STREAM(removeEntity,						MERCURY_VARIABLE_MESSAGE)

	// ��������ݿ�ɾ��ʵ��
	DBMGR_MESSAGE_DECLARE_STREAM(deleteBaseByDBID,					MERCURY_VARIABLE_MESSAGE)

	// ����رշ�����
	DBMGR_MESSAGE_DECLARE_STREAM(reqCloseServer,					MERCURY_VARIABLE_MESSAGE)

	// ĳ��app��app��֪���ڻ״̬��
	DBMGR_MESSAGE_DECLARE_ARGS6(queryEntity,						MERCURY_VARIABLE_MESSAGE, 
									COMPONENT_ID,					componentID,
									int8,							queryMode,
									DBID,							dbid, 
									std::string,					entityType,
									CALLBACK_ID,					callbackID,
									ENTITY_ID,						entityID)

	// ͬ��entity��ģ��
	DBMGR_MESSAGE_DECLARE_STREAM(syncEntityStreamTemplate,			MERCURY_VARIABLE_MESSAGE)

	// �����ѯwatcher����
	DBMGR_MESSAGE_DECLARE_STREAM(queryWatcher,						MERCURY_VARIABLE_MESSAGE)

	// ��ֵ����
	DBMGR_MESSAGE_DECLARE_STREAM(charge,							MERCURY_VARIABLE_MESSAGE)

	// ��ֵ�ص�
	DBMGR_MESSAGE_DECLARE_STREAM(onChargeCB,						MERCURY_VARIABLE_MESSAGE)

	// ����ص���
	DBMGR_MESSAGE_DECLARE_ARGS1(accountActivate,					MERCURY_VARIABLE_MESSAGE,
									std::string,					scode)

	// �˺������������롣
	DBMGR_MESSAGE_DECLARE_ARGS1(accountReqResetPassword,			MERCURY_VARIABLE_MESSAGE,
									std::string,					accountName)

	// �˺�����������롣
	DBMGR_MESSAGE_DECLARE_ARGS3(accountResetPassword,				MERCURY_VARIABLE_MESSAGE,
									std::string,					accountName,
									std::string,					newpassword,
									std::string,					code)

	// �˺���������䡣
	DBMGR_MESSAGE_DECLARE_ARGS4(accountReqBindMail,					MERCURY_VARIABLE_MESSAGE,
									ENTITY_ID,						entityID,
									std::string,					accountName,
									std::string,					password,
									std::string,					email)

	// �˺���ɰ����䡣
	DBMGR_MESSAGE_DECLARE_ARGS2(accountBindMail,					MERCURY_VARIABLE_MESSAGE,
									std::string,					username,
									std::string,					code)

	// �˺��޸����롣
	DBMGR_MESSAGE_DECLARE_ARGS4(accountNewPassword,					MERCURY_VARIABLE_MESSAGE,
									ENTITY_ID,						entityID,
									std::string,					accountName,
									std::string,					password,
									std::string,					newpassword)

	// ��ʼprofile
	DBMGR_MESSAGE_DECLARE_STREAM(startProfile,						MERCURY_VARIABLE_MESSAGE)

	// ����ǿ��ɱ����ǰapp
	DBMGR_MESSAGE_DECLARE_STREAM(reqKillServer,						MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
