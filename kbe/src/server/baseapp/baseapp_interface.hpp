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
	#undef KBE_BASEAPP_INTERFACE_HPP
#endif


#ifndef KBE_BASEAPP_INTERFACE_HPP
#define KBE_BASEAPP_INTERFACE_HPP

// common include	
#if defined(BASEAPP)
#include "baseapp.hpp"
#endif
#include "baseapp_interface_macros.hpp"
#include "base_interface_macros.hpp"
#include "proxy_interface_macros.hpp"
#include "network/interface_defs.hpp"
#include "server/server_errors.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	BASEAPP������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(BaseappInterface)
	// �ͻ���������Э�顣
	BASEAPP_MESSAGE_DECLARE_ARGS0(importClientMessages,								MERCURY_FIXED_MESSAGE)

	// �ͻ���entitydef������
	BASEAPP_MESSAGE_DECLARE_ARGS0(importClientEntityDef,							MERCURY_FIXED_MESSAGE)

	// ĳapp����������ߡ�
	BASEAPP_MESSAGE_DECLARE_ARGS0(reqClose,											MERCURY_FIXED_MESSAGE)

	// ĳapp��������look��
	BASEAPP_MESSAGE_DECLARE_ARGS0(lookApp,											MERCURY_FIXED_MESSAGE)

	// ĳ��app����鿴��app����״̬��
	BASEAPP_MESSAGE_DECLARE_ARGS0(queryLoad,										MERCURY_FIXED_MESSAGE)

	// consoleԶ��ִ��python��䡣
	BASEAPP_MESSAGE_DECLARE_STREAM(onExecScriptCommand,								MERCURY_VARIABLE_MESSAGE)

	// ĳappע���Լ��Ľӿڵ�ַ����app
	BASEAPP_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,								MERCURY_VARIABLE_MESSAGE,
									int32,											uid, 
									std::string,									username,
									int8,											componentType, 
									uint64,											componentID, 
									int8,											globalorderID,
									int8,											grouporderID,
									uint32,											intaddr, 
									uint16,											intport,
									uint32,											extaddr, 
									uint16,											extport,
									std::string,									extaddrEx)

	// dbmgr��֪�Ѿ�����������baseapp����cellapp�ĵ�ַ
	// ��ǰapp��Ҫ������ȥ�����ǽ�������
	BASEAPP_MESSAGE_DECLARE_ARGS11(onGetEntityAppFromDbmgr,							MERCURY_VARIABLE_MESSAGE,
									int32,											uid, 
									std::string,									username,
									int8,											componentType, 
									uint64,											componentID, 
									int8,											globalorderID,
									int8,											grouporderID,
									uint32,											intaddr, 
									uint16,											intport,
									uint32,											extaddr, 
									uint16,											extport,
									std::string,									extaddrEx)

	// ĳapp�����ȡһ��entityID�εĻص�
	BASEAPP_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,								MERCURY_FIXED_MESSAGE,
									ENTITY_ID,										startID,
									ENTITY_ID,										endID)


	// ĳapp�����ȡһ��entityID�εĻص�
	BASEAPP_MESSAGE_DECLARE_ARGS6(onDbmgrInitCompleted,								MERCURY_VARIABLE_MESSAGE,
									GAME_TIME,										gametime, 
									ENTITY_ID,										startID,
									ENTITY_ID,										endID,
									int32,											startGlobalOrder,
									int32,											startGroupOrder,
									std::string,									digest)

	// hello���֡�
	BASEAPP_MESSAGE_EXPOSED(hello)
	BASEAPP_MESSAGE_DECLARE_STREAM(hello,											MERCURY_VARIABLE_MESSAGE)

	// global���ݸı�
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChanged,					MERCURY_VARIABLE_MESSAGE)
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastBaseAppDataChanged,					MERCURY_VARIABLE_MESSAGE)

	// ĳ��app��app��֪���ڻ״̬��
	BASEAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,									MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,									componentType, 
									COMPONENT_ID,									componentID)

	// ĳ��app��app��֪���ڻ״̬��
	BASEAPP_MESSAGE_EXPOSED(onClientActiveTick)
	BASEAPP_MESSAGE_DECLARE_ARGS0(onClientActiveTick,								MERCURY_FIXED_MESSAGE)

	// �յ�baseappmgr������ĳ��baseappҪ��createBaseAnywhere�������ڱ�baseapp��ִ�� 
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseAnywhere,							MERCURY_VARIABLE_MESSAGE)

	// createBaseAnywhere�ɹ�֮��ص���Ϣ�������createBaseAnywhere��baseapp��entity��
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseAnywhereCallback,					MERCURY_FIXED_MESSAGE)

	// createCellEntity��cellʵ�崴���ɹ��ص���
	BASEAPP_MESSAGE_DECLARE_ARGS3(onEntityGetCell,									MERCURY_FIXED_MESSAGE,
									ENTITY_ID,										id,
									COMPONENT_ID,									componentID,
									SPACE_ID,										spaceID)

	// createCellEntity��cellʵ�崴���ɹ��ص���
	BASEAPP_MESSAGE_DECLARE_ARGS1(onCreateCellFailure,								MERCURY_FIXED_MESSAGE,
									ENTITY_ID,										entityID)

	// loginapp���Լ�ע��һ����Ҫ��¼���˺�, ��baseappmgrת����
	BASEAPP_MESSAGE_DECLARE_ARGS8(registerPendingLogin,								MERCURY_VARIABLE_MESSAGE,
									std::string,									loginName, 
									std::string,									accountName,
									std::string,									password,
									ENTITY_ID,										entityID,
									DBID,											entityDBID,
									uint32,											flags,
									uint64,											deadline,
									COMPONENT_TYPE,									componentType)

	// ǰ�������¼�������ϡ�
	BASEAPP_MESSAGE_EXPOSED(loginGateway)
	BASEAPP_MESSAGE_DECLARE_ARGS2(loginGateway,										MERCURY_VARIABLE_MESSAGE,
									std::string,									accountName,
									std::string,									password)

	// ǰ���������µ�¼�������ϡ�
	BASEAPP_MESSAGE_EXPOSED(reLoginGateway)
	BASEAPP_MESSAGE_DECLARE_ARGS4(reLoginGateway,									MERCURY_VARIABLE_MESSAGE,
									std::string,									accountName,
									std::string,									password,
									uint64,											key,
									ENTITY_ID,										entityID)

	// ��dbmgr��ȡ���˺�Entity��Ϣ
	BASEAPP_MESSAGE_DECLARE_STREAM(onQueryAccountCBFromDbmgr,						MERCURY_VARIABLE_MESSAGE)

	// entity�յ�һ��mail, ��ĳ��app�ϵ�mailbox����
	BASEAPP_MESSAGE_DECLARE_STREAM(onEntityMail,									MERCURY_VARIABLE_MESSAGE)
	
	// client����entity��cell����
	BASEAPP_MESSAGE_EXPOSED(onRemoteCallCellMethodFromClient)
	BASEAPP_MESSAGE_DECLARE_STREAM(onRemoteCallCellMethodFromClient,				MERCURY_VARIABLE_MESSAGE)

	// client��������
	BASEAPP_MESSAGE_EXPOSED(onUpdateDataFromClient)
	BASEAPP_MESSAGE_DECLARE_STREAM(onUpdateDataFromClient,							MERCURY_VARIABLE_MESSAGE)

	// executeRawDatabaseCommand��dbmgr�Ļص�
	BASEAPP_MESSAGE_DECLARE_STREAM(onExecuteRawDatabaseCommandCB,					MERCURY_VARIABLE_MESSAGE)

	// cellapp����entity��cell����
	BASEAPP_MESSAGE_DECLARE_STREAM(onBackupEntityCellData,							MERCURY_VARIABLE_MESSAGE)

	// cellapp writeToDB���
	BASEAPP_MESSAGE_DECLARE_STREAM(onCellWriteToDBCompleted,						MERCURY_VARIABLE_MESSAGE)

	// cellappת��entity��Ϣ��client
	BASEAPP_MESSAGE_DECLARE_STREAM(forwardMessageToClientFromCellapp,				MERCURY_VARIABLE_MESSAGE)

	// cellappת��entity��Ϣ��ĳ��baseEntity��cellEntity
	BASEAPP_MESSAGE_DECLARE_STREAM(forwardMessageToCellappFromCellapp,				MERCURY_VARIABLE_MESSAGE)

	// ����رշ�����
	BASEAPP_MESSAGE_DECLARE_STREAM(reqCloseServer,									MERCURY_VARIABLE_MESSAGE)

	// дentity��db�ص���
	BASEAPP_MESSAGE_DECLARE_ARGS4(onWriteToDBCallback,								MERCURY_FIXED_MESSAGE,
									ENTITY_ID,										eid,
									DBID,											entityDBID,
									CALLBACK_ID,									callbackID,
									bool,											success)

	// createBaseFromDBID�Ļص�
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseFromDBIDCallback,					MERCURY_FIXED_MESSAGE)

	// createBaseAnywhereFromDBID�Ļص�
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseAnywhereFromDBIDCallback,			MERCURY_FIXED_MESSAGE)

	// createBaseAnywhereFromDBID�Ļص�
	BASEAPP_MESSAGE_DECLARE_STREAM(createBaseAnywhereFromDBIDOtherBaseapp,			MERCURY_FIXED_MESSAGE)

	// createBaseAnywhereFromDBID�Ļص�
	BASEAPP_MESSAGE_DECLARE_ARGS5(onCreateBaseAnywhereFromDBIDOtherBaseappCallback,	MERCURY_VARIABLE_MESSAGE,
									COMPONENT_ID,									createByBaseappID,
									std::string,									entityType,
									ENTITY_ID,										createdEntityID,
									CALLBACK_ID,									callbackID,
									DBID,											dbid)

	// �����ѯwatcher����
	BASEAPP_MESSAGE_DECLARE_STREAM(queryWatcher,									MERCURY_VARIABLE_MESSAGE)

	// ��ֵ�ص�
	BASEAPP_MESSAGE_DECLARE_STREAM(onChargeCB,										MERCURY_VARIABLE_MESSAGE)

	// ��ʼprofile
	BASEAPP_MESSAGE_DECLARE_STREAM(startProfile,									MERCURY_VARIABLE_MESSAGE)

	// ��������ݿ�ɾ��ʵ��
	BASEAPP_MESSAGE_DECLARE_STREAM(deleteBaseByDBIDCB,								MERCURY_VARIABLE_MESSAGE)
	
	// ĳ��baseapp�ϵ�space�ָ���cell�� �жϵ�ǰbaseapp�Ƿ������entity��Ҫ�ָ�cell
	BASEAPP_MESSAGE_DECLARE_STREAM(onRestoreSpaceCellFromOtherBaseapp,				MERCURY_VARIABLE_MESSAGE)

	// �����email
	BASEAPP_MESSAGE_EXPOSED(reqAccountBindEmail)
	BASEAPP_MESSAGE_DECLARE_ARGS3(reqAccountBindEmail,								MERCURY_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									password,
									std::string,									email)

	// �����email����Ļص�
	BASEAPP_MESSAGE_DECLARE_ARGS5(onReqAccountBindEmailCB,							MERCURY_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									accountName,
									std::string,									email,
									SERVER_ERROR_CODE,								failedcode,
									std::string,									code)

	// �����޸�����
	BASEAPP_MESSAGE_EXPOSED(reqAccountNewPassword)
	BASEAPP_MESSAGE_DECLARE_ARGS3(reqAccountNewPassword,							MERCURY_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									oldpassword,
									std::string,									newpassword)

	// �����޸�����Ļص�
	BASEAPP_MESSAGE_DECLARE_ARGS3(onReqAccountNewPasswordCB,						MERCURY_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									accountName,
									SERVER_ERROR_CODE,								failedcode)

	// ����ǿ��ɱ����ǰapp
	BASEAPP_MESSAGE_DECLARE_STREAM(reqKillServer,									MERCURY_VARIABLE_MESSAGE)

	//--------------------------------------------Base----------------------------------------------------------
	// Զ�̺���entity����
	BASE_MESSAGE_EXPOSED(onRemoteMethodCall)
	BASE_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,									MERCURY_VARIABLE_MESSAGE)

	// cellappͨ����entity��cell�������ٻ��߶�ʧ
	BASE_MESSAGE_DECLARE_STREAM(onLoseCell,											MERCURY_VARIABLE_MESSAGE)

	// �ͻ���ֱ�ӷ�����Ϣ��cellʵ��
	BASE_MESSAGE_EXPOSED(forwardEntityMessageToCellappFromClient)
	BASE_MESSAGE_DECLARE_STREAM(forwardEntityMessageToCellappFromClient,			MERCURY_VARIABLE_MESSAGE)
	
	// ĳ��entity����teleport����entity��space��
	BASE_MESSAGE_DECLARE_ARGS3(reqTeleportOther,									MERCURY_FIXED_MESSAGE,
								ENTITY_ID,											reqTeleportEntityID,
								COMPONENT_ID,										reqTeleportEntityAppID,
								COMPONENT_ID,										reqTeleportEntityBaseAppID)

	// ĳ��entity����teleport��Ļص����
	BASE_MESSAGE_DECLARE_ARGS2(onTeleportCB,										MERCURY_FIXED_MESSAGE,
								SPACE_ID,											spaceID,
								bool,												fromCellTeleport)

	// ĳ��entity����teleport��Ļص����
	BASE_MESSAGE_DECLARE_ARGS1(onGetDBID,											MERCURY_FIXED_MESSAGE,
								DBID,												dbid)

	// entity����Ǩ�Ƶ���һ��cellapp�ϵ�space���̿�ʼ
	BASE_MESSAGE_DECLARE_ARGS1(onMigrationCellappStart,								MERCURY_FIXED_MESSAGE,
								COMPONENT_ID,										cellAppID)
	
	// entity����Ǩ�Ƶ���һ��cellapp�ϵ�space���̽���
	BASE_MESSAGE_DECLARE_ARGS1(onMigrationCellappEnd,								MERCURY_FIXED_MESSAGE,
								COMPONENT_ID,										cellAppID)

	//--------------------------------------------Proxy---------------------------------------------------------
	/**
		Զ�̺���entity����
	*/
	//PROXY_MESSAGE_EXPOSED(onClientGetCell)
	//PROXY_MESSAGE_DECLARE_ARGS0(onClientGetCell,									MERCURY_FIXED_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
