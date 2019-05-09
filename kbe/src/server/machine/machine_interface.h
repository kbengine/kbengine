// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_MACHINE_INTERFACE_H
#endif


#ifndef KBE_MACHINE_INTERFACE_H
#define KBE_MACHINE_INTERFACE_H

// common include	
#if defined(MACHINE)
#include "machine.h"
#endif
#include "machine_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	machine������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(MachineInterface)
	// ���������app�㲥�Լ��Ľӿڵ�ַ
	MACHINE_MESSAGE_DECLARE_ARGS25(onBroadcastInterface,			NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID, 
									COMPONENT_ID,					componentIDEx, 
									COMPONENT_ORDER,				globalorderid, 
									COMPONENT_ORDER,				grouporderid, 
									COMPONENT_GUS,					gus,
									uint32,							intaddr, 
									uint16,							intport,
									uint32,							extaddr, 
									uint16,							extport,
									std::string,					extaddrEx,
									uint32,							pid,
									float,							cpu, 
									float,							mem, 
									uint32,							usedmem,
									int8,							state,
									uint32,							machineID, 
									uint64,							extradata,
									uint64,							extradata1,
									uint64,							extradata2,
									uint64,							extradata3,
									uint32,							backRecvAddr,
									uint16,							backRecvPort)
	
	// ���������app�����ȡĳ��������ĵ�ַ
	MACHINE_MESSAGE_DECLARE_ARGS7(onFindInterfaceAddr,				NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID, 
									COMPONENT_TYPE,					findComponentType,
									uint32,							addr, 
									uint16,							finderRecvPort)
						
	// ��ѯ���нӿ���Ϣ
	MACHINE_MESSAGE_DECLARE_ARGS3(onQueryAllInterfaceInfos,			NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									uint16,							finderRecvPort)
		
	// ��ѯ����machine����
	MACHINE_MESSAGE_DECLARE_ARGS3(onQueryMachines,					NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									uint16,							finderRecvPort)

	MACHINE_MESSAGE_DECLARE_ARGS6(queryComponentID,					NETWORK_VARIABLE_MESSAGE,
									COMPONENT_TYPE,					componentType,
									COMPONENT_ID,					componentID,
									int32,							uid,
									uint16,							finderRecvPort,
									int,							macMD5,
									int32,							pid)
	// ĳapp��������look��
	MACHINE_MESSAGE_DECLARE_ARGS0(lookApp,							NETWORK_FIXED_MESSAGE)

	// ĳ��app����鿴��app����״̬��
	MACHINE_MESSAGE_DECLARE_ARGS0(queryLoad,						NETWORK_FIXED_MESSAGE)

	// ����������
	MACHINE_MESSAGE_DECLARE_STREAM(startserver,						NETWORK_VARIABLE_MESSAGE)

	// �رշ�����
	MACHINE_MESSAGE_DECLARE_STREAM(stopserver,						NETWORK_VARIABLE_MESSAGE)

	// �رշ�����
	MACHINE_MESSAGE_DECLARE_STREAM(killserver,						NETWORK_VARIABLE_MESSAGE)

	// ����ǿ��ɱ����ǰapp
	MACHINE_MESSAGE_DECLARE_STREAM(reqKillServer,					NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
