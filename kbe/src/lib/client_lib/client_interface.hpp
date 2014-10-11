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
	#undef KBE_CLIENT_INTERFACE_HPP
#endif


#ifndef KBE_CLIENT_INTERFACE_HPP
#define KBE_CLIENT_INTERFACE_HPP

// common include	
#if defined(CLIENT)
#include "clientapp.hpp"
#endif
#include "client_interface_macros.hpp"
#include "network/interface_defs.hpp"
#include "server/server_errors.hpp"
#include "entitydef/common.hpp"
#include "common.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	CLIENT������Ϣ�ӿ��ڴ˶���
*/
NETWORK_INTERFACE_DECLARE_BEGIN(ClientInterface)
	// �����hello���ء�
	CLIENT_MESSAGE_DECLARE_STREAM(onHelloCB,								MERCURY_VARIABLE_MESSAGE)

	// �ͷ���˵İ汾��ƥ��
	CLIENT_MESSAGE_DECLARE_STREAM(onVersionNotMatch,						MERCURY_VARIABLE_MESSAGE)

	// �ͷ���˵Ľű���汾��ƥ��
	CLIENT_MESSAGE_DECLARE_STREAM(onScriptVersionNotMatch,					MERCURY_VARIABLE_MESSAGE)

	// �����˺�ʧ�ܡ�
	CLIENT_MESSAGE_DECLARE_STREAM(onCreateAccountResult,					MERCURY_VARIABLE_MESSAGE)

	// ��¼�ɹ���
	CLIENT_MESSAGE_DECLARE_STREAM(onLoginSuccessfully,						MERCURY_VARIABLE_MESSAGE)

	// ��¼ʧ�ܡ�
	CLIENT_MESSAGE_DECLARE_STREAM(onLoginFailed,							MERCURY_VARIABLE_MESSAGE)

	// ���������Ѿ�������һ����ͻ��˹����Ĵ���Entity || ��¼���سɹ���
	CLIENT_MESSAGE_DECLARE_ARGS3(onCreatedProxies,							MERCURY_VARIABLE_MESSAGE,
									uint64,									rndUUID,
									ENTITY_ID,								eid,
									std::string,							entityType)

	// ��¼����ʧ�ܡ�
	CLIENT_MESSAGE_DECLARE_ARGS1(onLoginGatewayFailed,						MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// ��¼����ʧ�ܡ�
	CLIENT_MESSAGE_DECLARE_ARGS1(onReLoginGatewayFailed,					MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// �������ϵ�entity�Ѿ�������Ϸ�����ˡ�
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityEnterWorld,						MERCURY_VARIABLE_MESSAGE)

	// �������ϵ�entity�Ѿ��뿪��Ϸ�����ˡ�
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityLeaveWorld,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// �������ϵ�entity�Ѿ��뿪��Ϸ�����ˡ�
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityLeaveWorldOptimized,				MERCURY_VARIABLE_MESSAGE)

	// ���߿ͻ���ĳ��entity�����ˣ� ����entityͨ���ǻ�δonEntityEnterWorld��
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityDestroyed,							MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// �������ϵ�entity�Ѿ�����space�ˡ�
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityEnterSpace,						MERCURY_VARIABLE_MESSAGE)

	// �������ϵ�entity�Ѿ��뿪space�ˡ�
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityLeaveSpace,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// Զ�̺���entity����
	CLIENT_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onRemoteMethodCallOptimized,				MERCURY_VARIABLE_MESSAGE)

	// ���߳�������
	CLIENT_MESSAGE_DECLARE_ARGS1(onKicked,									MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// ����������entity����
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdatePropertys,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdatePropertysOptimized,				MERCURY_VARIABLE_MESSAGE)

	// ������ǿ������entity��λ���볯��
	CLIENT_MESSAGE_DECLARE_STREAM(onSetEntityPosAndDir,						MERCURY_VARIABLE_MESSAGE)

	// ���������°�
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateBasePos,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateBasePosXZ,						MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData,								MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_ypr,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yp,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yr,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_pr,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_y,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_p,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_r,							MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_ypr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yp,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_pr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_y,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_p,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_r,						MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_ypr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yp,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_pr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_y,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_p,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_r,						MERCURY_VARIABLE_MESSAGE)

	// download stream��ʼ�� 
	CLIENT_MESSAGE_DECLARE_ARGS3(onStreamDataStarted,						MERCURY_VARIABLE_MESSAGE,
									int16,									id,
									uint32,									datasize,
									std::string,							descr)

	// ���յ�streamData
	CLIENT_MESSAGE_DECLARE_STREAM(onStreamDataRecv,							MERCURY_VARIABLE_MESSAGE)

	// download stream����� 
	CLIENT_MESSAGE_DECLARE_ARGS1(onStreamDataCompleted,						MERCURY_FIXED_MESSAGE,
									int16,									id)

	// ����Э��
	CLIENT_MESSAGE_DECLARE_STREAM(onImportClientMessages,					MERCURY_VARIABLE_MESSAGE)
	
	// ����entitydef
	CLIENT_MESSAGE_DECLARE_STREAM(onImportClientEntityDef,					MERCURY_VARIABLE_MESSAGE)

	// ��������������
	CLIENT_MESSAGE_DECLARE_STREAM(onImportServerErrorsDescr,				MERCURY_VARIABLE_MESSAGE)

	// ����˳�ʼ��spacedata
	CLIENT_MESSAGE_DECLARE_STREAM(initSpaceData,							MERCURY_VARIABLE_MESSAGE)

	// �����������spacedata
	CLIENT_MESSAGE_DECLARE_ARGS3(setSpaceData,								MERCURY_VARIABLE_MESSAGE,
									SPACE_ID,								spaceID,
									std::string,							key,
									std::string,							valye)

	// �����ɾ����spacedata
	CLIENT_MESSAGE_DECLARE_ARGS2(delSpaceData,								MERCURY_VARIABLE_MESSAGE,
									SPACE_ID,								spaceID,
									std::string,							key)

	// �����˺��������󷵻�
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountResetPasswordCB,				MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// �����˺��������󷵻�
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountBindEmailCB,					MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// �����˺��������󷵻�
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountNewPasswordCB,					MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// �ص�½���سɹ� 
	CLIENT_MESSAGE_DECLARE_STREAM(onReLoginGatewaySuccessfully,				MERCURY_VARIABLE_MESSAGE)
									
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}

#endif // KBE_CLIENT_INTERFACE_HPP
