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
	#undef KBE_CELLAPP_INTERFACE_HPP
#endif


#ifndef KBE_CELLAPP_INTERFACE_HPP
#define KBE_CELLAPP_INTERFACE_HPP

// common include	
#if defined(CELLAPP)
#include "entity.hpp"
#include "cellapp.hpp"
#endif
#include "cellapp_interface_macros.hpp"
#include "entity_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	cellapp������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(CellappInterface)
	// ĳappע���Լ��Ľӿڵ�ַ����app
	CELLAPP_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,						MERCURY_VARIABLE_MESSAGE,
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

	// ĳapp��������look��
	CELLAPP_MESSAGE_DECLARE_ARGS0(lookApp,									MERCURY_FIXED_MESSAGE)

	// ĳ��app����鿴��app����״̬��
	CELLAPP_MESSAGE_DECLARE_ARGS0(queryLoad,								MERCURY_FIXED_MESSAGE)

	// consoleԶ��ִ��python��䡣
	CELLAPP_MESSAGE_DECLARE_STREAM(onExecScriptCommand,						MERCURY_VARIABLE_MESSAGE)

	// dbmgr��֪�Ѿ�����������baseapp����cellapp�ĵ�ַ
	// ��ǰapp��Ҫ������ȥ�����ǽ�������
	CELLAPP_MESSAGE_DECLARE_ARGS11(onGetEntityAppFromDbmgr,					MERCURY_VARIABLE_MESSAGE,
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

	// ĳapp�����ȡһ��entityID�εĻص�
	CELLAPP_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								startID,
									ENTITY_ID,								endID)

	// ĳapp�����ȡһ��entityID�εĻص�
	CELLAPP_MESSAGE_DECLARE_ARGS6(onDbmgrInitCompleted,						MERCURY_VARIABLE_MESSAGE,
									GAME_TIME,								gametime, 
									ENTITY_ID,								startID,
									ENTITY_ID,								endID,
									int32,									startGlobalOrder,
									int32,									startGroupOrder,
									std::string,							digest)

	// global���ݸı�
	CELLAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChanged,			MERCURY_VARIABLE_MESSAGE)
	CELLAPP_MESSAGE_DECLARE_STREAM(onBroadcastCellAppDataChanged,			MERCURY_VARIABLE_MESSAGE)

	// baseEntity���󴴽���һ���µ�space�С�
	CELLAPP_MESSAGE_DECLARE_STREAM(onCreateInNewSpaceFromBaseapp,			MERCURY_VARIABLE_MESSAGE)

	// baseEntity����ָ���һ���µ�space�С�
	CELLAPP_MESSAGE_DECLARE_STREAM(onRestoreSpaceInCellFromBaseapp,			MERCURY_VARIABLE_MESSAGE)

	// baseapp���������cellapp�ϴ���һ��entity��
	CELLAPP_MESSAGE_DECLARE_STREAM(onCreateCellEntityFromBaseapp,			MERCURY_VARIABLE_MESSAGE)

	// ����ĳ��cellEntity��
	CELLAPP_MESSAGE_DECLARE_ARGS1(onDestroyCellEntityFromBaseapp,			MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// ĳ��app��app��֪���ڻ״̬��
	CELLAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,							MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID)

	// entity�յ�һ��mail, ��ĳ��app�ϵ�mailbox����
	CELLAPP_MESSAGE_DECLARE_STREAM(onEntityMail,							MERCURY_VARIABLE_MESSAGE)

	// client����entity��cell����
	CELLAPP_MESSAGE_DECLARE_STREAM(onRemoteCallMethodFromClient,			MERCURY_VARIABLE_MESSAGE)

	// client��������
	CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateDataFromClient,					MERCURY_VARIABLE_MESSAGE)

	// executeRawDatabaseCommand��dbmgr�Ļص�
	CELLAPP_MESSAGE_DECLARE_STREAM(onExecuteRawDatabaseCommandCB,			MERCURY_VARIABLE_MESSAGE)

	// base�����ȡcelldata
	CELLAPP_MESSAGE_DECLARE_STREAM(reqBackupEntityCellData,					MERCURY_VARIABLE_MESSAGE)

	// base�����ȡWriteToDB
	CELLAPP_MESSAGE_DECLARE_STREAM(reqWriteToDBFromBaseapp,					MERCURY_VARIABLE_MESSAGE)

	// �ͻ���ֱ�ӷ�����Ϣ��cellʵ��
	CELLAPP_MESSAGE_DECLARE_STREAM(forwardEntityMessageToCellappFromClient,	MERCURY_VARIABLE_MESSAGE)

	// ����رշ�����
	CELLAPP_MESSAGE_DECLARE_STREAM(reqCloseServer,							MERCURY_VARIABLE_MESSAGE)

	// �����ѯwatcher����
	CELLAPP_MESSAGE_DECLARE_STREAM(queryWatcher,							MERCURY_VARIABLE_MESSAGE)

	// ��ʼprofile
	CELLAPP_MESSAGE_DECLARE_STREAM(startProfile,							MERCURY_VARIABLE_MESSAGE)

	// ����teleport����ǰcellapp��
	CELLAPP_MESSAGE_DECLARE_STREAM(reqTeleportToTheCellApp,					MERCURY_VARIABLE_MESSAGE)

	// entity���͵�Ŀ��cellapp�ϵ�space֮�� ���ظ�֮ǰcellapp�Ļص�
	CELLAPP_MESSAGE_DECLARE_STREAM(reqTeleportToTheCellAppCB,				MERCURY_VARIABLE_MESSAGE)

	// real����������Ե�ghost
	CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateGhostPropertys,					MERCURY_VARIABLE_MESSAGE)
	
	// ghost�������def����real
	CELLAPP_MESSAGE_DECLARE_STREAM(onRemoteRealMethodCall,					MERCURY_VARIABLE_MESSAGE)

	// real��������ױ����ݵ�ghost
	CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateGhostVolatileData,				MERCURY_VARIABLE_MESSAGE)

	// ����ǿ��ɱ����ǰapp
	CELLAPP_MESSAGE_DECLARE_STREAM(reqKillServer,							MERCURY_VARIABLE_MESSAGE)

	//--------------------------------------------Entity----------------------------------------------------------
	//Զ�̺���entity����
	ENTITY_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,						MERCURY_VARIABLE_MESSAGE)

	//�ͻ���������λ��
	ENTITY_MESSAGE_DECLARE_ARGS2(setPosition_XZ_int,						MERCURY_FIXED_MESSAGE,
									int32,									x, 
									int32,									z)
	
	//�ͻ���������λ��
	ENTITY_MESSAGE_DECLARE_ARGS3(setPosition_XYZ_int,						MERCURY_FIXED_MESSAGE,
									int32,									x, 
									int32,									y, 
									int32,									z)

	//�ͻ���������λ��
	ENTITY_MESSAGE_DECLARE_ARGS2(setPosition_XZ_float,						MERCURY_FIXED_MESSAGE,
									float,									x, 
									float,									z)

	//�ͻ���������λ��
	ENTITY_MESSAGE_DECLARE_ARGS3(setPosition_XYZ_float,						MERCURY_FIXED_MESSAGE,
									float,									x, 
									float,									y, 
									float,									z)

	//entity����һ���۲���(�ͻ���)
	ENTITY_MESSAGE_DECLARE_ARGS0(onGetWitnessFromBase,						MERCURY_FIXED_MESSAGE)

	//entity��ʧ��һ���۲���(�ͻ���)
	ENTITY_MESSAGE_DECLARE_ARGS0(onLoseWitness,								MERCURY_FIXED_MESSAGE)

	// entity���͡�
	ENTITY_MESSAGE_DECLARE_ARGS3(teleportFromBaseapp,						MERCURY_FIXED_MESSAGE,
									COMPONENT_ID,							cellAppID,
									ENTITY_ID,								targetEntityID,
									COMPONENT_ID,							sourceBaseAppID)
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
