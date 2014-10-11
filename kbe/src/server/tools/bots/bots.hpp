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


#ifndef KBE_BOTS_HPP
#define KBE_BOTS_HPP
	
// common include	
#include "profile.hpp"
#include "create_and_login_handler.hpp"
#include "cstdkbe/timer.hpp"
#include "pyscript/script.hpp"
#include "network/endpoint.hpp"
#include "helper/debug_helper.hpp"
#include "helper/script_loglevel.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_poller.hpp"
#include "client_lib/clientapp.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "entitydef/entitydef.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class ClientObject;
class PyBots;
class TelnetServer;
typedef SmartPointer<ClientObject> ClientObjectPtr;

class Bots  : public ClientApp
{
public:
	Bots(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Bots();

	virtual bool initialize();
	virtual void finalise();

	virtual bool initializeBegin();
	virtual bool initializeEnd();

	virtual bool installPyModules();
	virtual void onInstallPyModules() {};
	virtual bool uninstallPyModules();
	bool uninstallPyScript();
	bool installEntityDef();

	virtual void handleTimeout(TimerHandle, void * pUser);
	virtual void handleGameTick();

	static Bots& getSingleton(){ 
		return *static_cast<Bots*>(ClientApp::getSingletonPtr()); 
	}

	/**
		���ýű��������ǰ׺
	*/
	static PyObject* __py_setScriptLogType(PyObject* self, PyObject* args);

	bool run(void);

	/**
		��mailbox�����Ի�ȡһ��channel��ʵ��
	*/
	virtual Mercury::Channel* findChannelByMailbox(EntityMailbox& mailbox);

	/** ����ӿ�
		ĳ��app����鿴��app
	*/
	virtual void lookApp(Mercury::Channel* pChannel);

	/** ����ӿ�
		����رշ�����
	*/
	virtual void reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		����رշ�����
	*/
	void reqKillServer(Mercury::Channel* pChannel, MemoryStream& s);

	void onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	typedef std::map< Mercury::Channel*, ClientObjectPtr > CLIENTS;
	CLIENTS& clients(){ return clients_; }

	uint32 reqCreateAndLoginTotalCount(){ return reqCreateAndLoginTotalCount_; }
	void reqCreateAndLoginTotalCount(uint32 v){ reqCreateAndLoginTotalCount_ = v; }

	uint32 reqCreateAndLoginTickCount(){ return reqCreateAndLoginTickCount_; }
	void reqCreateAndLoginTickCount(uint32 v){ reqCreateAndLoginTickCount_ = v; }

	float reqCreateAndLoginTickTime(){ return reqCreateAndLoginTickTime_; }
	void reqCreateAndLoginTickTime(float v){ reqCreateAndLoginTickTime_ = v; }

	bool addClient(ClientObject* pClient);
	bool delClient(ClientObject* pClient);

	ClientObject* findClient(Mercury::Channel * pChannel);
	ClientObject* findClientByAppID(int32 appID);

	static PyObject* __py_addBots(PyObject* self, PyObject* args);

	/** ����ӿ�
	   ���bots
	   @total uint32: �ܹ���ӵĸ���
	   @ticknum uint32: ÿ��tick��Ӷ��ٸ�
	   @ticktime float: һ��tick��ʱ��
	*/
	virtual void addBots(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
		ĳ��app��app��֪���ڻ״̬��
	*/
	void onAppActiveTick(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID);

	virtual void onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo,
		const std::string& scriptVerInfo, const std::string& protocolMD5, 
		const std::string& entityDefMD5, COMPONENT_TYPE componentType);

	/** ����ӿ�
		�ͷ���˵İ汾��ƥ��
	*/
	virtual void onVersionNotMatch(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		�ͷ���˵Ľű���汾��ƥ��
	*/
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		�����˺ųɹ���ʧ�ܻص�
	   @failedcode: ʧ�ܷ����� MERCURY_ERR_SRV_NO_READY:������û��׼����, 
									MERCURY_ERR_ACCOUNT_CREATE:����ʧ�ܣ��Ѿ����ڣ�, 
									MERCURY_SUCCESS:�˺Ŵ����ɹ�

									SERVER_ERROR_CODE failedcode;
		@�����Ƹ�������:�����ƶ�������: uint32���� + bytearray
	*/
	virtual void onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s);

	Mercury::EventPoller* pEventPoller(){ return pEventPoller_; }

	/** ����ӿ�
	   ��¼ʧ�ܻص�
	   @failedcode: ʧ�ܷ����� MERCURY_ERR_SRV_NO_READY:������û��׼����, 
									MERCURY_ERR_SRV_OVERLOAD:���������ع���, 
									MERCURY_ERR_NAME_PASSWORD:�û����������벻��ȷ
	*/
	virtual void onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
	   ��¼�ɹ�
	   @ip: ������ip��ַ
	   @port: �������˿�
	*/
	virtual void onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
	   ��¼ʧ�ܻص�
	   @failedcode: ʧ�ܷ����� MERCURY_ERR_SRV_NO_READY:������û��׼����, 
									MERCURY_ERR_ILLEGAL_LOGIN:�Ƿ���¼, 
									MERCURY_ERR_NAME_PASSWORD:�û����������벻��ȷ
	*/
	virtual void onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** ����ӿ�
	   �ص�½baseapp�ɹ�
	*/
	virtual void onReLoginGatewaySuccessfully(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
		���������Ѿ�������һ����ͻ��˹����Ĵ���Entity
	   �ڵ�¼ʱҲ�ɱ��ɹ��ص�
	   @datas: �˺�entity����Ϣ
	*/
	virtual void onCreatedProxies(Mercury::Channel * pChannel, uint64 rndUUID, 
		ENTITY_ID eid, std::string& entityType);

	/** ����ӿ�
		�������ϵ�entity�Ѿ�������Ϸ������
	*/
	virtual void onEntityEnterWorld(Mercury::Channel * pChannel, MemoryStream& s);


	/** ����ӿ�
		�������ϵ�entity�Ѿ��뿪��Ϸ������
	*/
	virtual void onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid);
	virtual void onEntityLeaveWorldOptimized(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
		���߿ͻ���ĳ��entity�����ˣ� ����entityͨ���ǻ�δonEntityEnterWorld
	*/
	virtual void onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid);

	/** ����ӿ�
		�������ϵ�entity�Ѿ�����space��
	*/
	virtual void onEntityEnterSpace(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
		�������ϵ�entity�Ѿ��뿪space��
	*/
	virtual void onEntityLeaveSpace(Mercury::Channel * pChannel, ENTITY_ID eid);

	/** ����ӿ�
		Զ�̵���entity�ķ��� 
	*/
	virtual void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onRemoteMethodCallOptimized(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
	   ���߳�������
	*/
	virtual void onKicked(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** ����ӿ�
		����������entity����
	*/
	virtual void onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdatePropertysOptimized(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		����������avatar����λ��
	*/
	virtual void onUpdateBasePos(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateBasePosXZ(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		������ǿ������entity��λ���볯��
	*/
	virtual void onSetEntityPosAndDir(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		����������VolatileData
	*/
	virtual void onUpdateData(Mercury::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_ypr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yp(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_yr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_pr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_y(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_p(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_r(Mercury::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xz(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_ypr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yp(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_yr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_pr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_y(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_p(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xz_r(Mercury::Channel* pChannel, MemoryStream& s);

	virtual void onUpdateData_xyz(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_ypr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yp(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_yr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_pr(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_y(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_p(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateData_xyz_r(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		download stream��ʼ�� 
	*/
	virtual void onStreamDataStarted(Mercury::Channel* pChannel, int16 id, uint32 datasize, std::string& descr);

	/** ����ӿ�
		���յ�streamData
	*/
	virtual void onStreamDataRecv(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		download stream����� 
	*/
	virtual void onStreamDataCompleted(Mercury::Channel* pChannel, int16 id);

	/** ����ӿ�
		space��ز����ӿ�
		����������ĳ��space�ļ���ӳ��
	*/
	void setSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value);
	void delSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key);

	/** ����ӿ�
		����鿴watcher
	*/
	void queryWatcher(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		console����ʼprofile
	*/
	void startProfile(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	virtual void startProfile_(Mercury::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen);

protected:
	PyBots*													pPyBots_;

	CLIENTS													clients_;

	// console���󴴽�������˵�bots����
	uint32													reqCreateAndLoginTotalCount_;
	uint32													reqCreateAndLoginTickCount_;
	float													reqCreateAndLoginTickTime_;

	// ���������¼��handler
	CreateAndLoginHandler*									pCreateAndLoginHandler_;

	Mercury::EventPoller*									pEventPoller_;

	TelnetServer*											pTelnetServer_;
};

}

#endif // KBE_BOTS_HPP
