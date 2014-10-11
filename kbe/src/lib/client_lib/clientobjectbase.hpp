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


#ifndef CLIENT_OBJECT_BASE_HPP
#define CLIENT_OBJECT_BASE_HPP

#include "event.hpp"
#include "script_callbacks.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"
#include "helper/script_loglevel.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/entities.hpp"
#include "entitydef/common.hpp"
#include "server/callbackmgr.hpp"
#include "server/server_errors.hpp"
#include "math/math.hpp"

namespace KBEngine{

namespace client{
class Entity;
}

class EntityMailbox;

namespace Mercury
{
class Channel;
}

class ClientObjectBase : public script::ScriptObject
{
	/** 
		���໯ ��һЩpy�������������� 
	*/
	INSTANCE_SCRIPT_HREADER(ClientObjectBase, ScriptObject)	
public:
	ClientObjectBase(Mercury::NetworkInterface& ninterface, PyTypeObject* pyType = NULL);
	virtual ~ClientObjectBase();

	Mercury::Channel* pServerChannel()const{ return pServerChannel_; }

	void finalise(void);
	virtual void reset(void);
	void canReset(bool v){ canReset_ = v; }

	Entities<client::Entity>* pEntities()const{ return pEntities_; }

	/**
		����һ��entity 
	*/
	client::Entity* createEntityCommon(const char* entityType, PyObject* params,
		bool isInitializeScript = true, ENTITY_ID eid = 0, bool initProperty = true, 
		EntityMailbox* base = NULL, EntityMailbox* cell = NULL);

	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }	

	/**
		ͨ��entityID����һ��entity 
	*/
	virtual bool destroyEntity(ENTITY_ID entityID, bool callScript);

	void tickSend();
	
	virtual Mercury::Channel* initLoginappChannel(std::string accountName, 
		std::string passwd, std::string ip, KBEngine::uint32 port);
	virtual Mercury::Channel* initBaseappChannel();

	bool createAccount();
	bool login();
	
	bool loginGateWay();
	bool reLoginGateWay();

	int32 appID()const{ return appID_; }
	const char* name(){ return name_.c_str(); }

	ENTITY_ID entityID(){ return entityID_; }
	DBID dbid(){ return dbid_; }

	bool registerEventHandle(EventHandle* pEventHandle);
	bool deregisterEventHandle(EventHandle* pEventHandle);
	
	void fireEvent(const EventData* pEventData);
	
	EventHandler& eventHandler(){ return eventHandler_; }

	static PyObject* __pyget_pyGetEntities(PyObject *self, void *closure)
	{
		ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);
		Py_INCREF(pClientObjectBase->pEntities());
		return pClientObjectBase->pEntities(); 
	}

	static PyObject* __pyget_pyGetID(PyObject *self, void *closure){
		
		ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);
		return PyLong_FromLong(pClientObjectBase->appID());	
	}
	
	static PyObject* __py_callback(PyObject* self, PyObject* args);
	static PyObject* __py_cancelCallback(PyObject* self, PyObject* args);

	static PyObject* __py_getWatcher(PyObject* self, PyObject* args);
	static PyObject* __py_getWatcherDir(PyObject* self, PyObject* args);

	static PyObject* __py_disconnect(PyObject* self, PyObject* args);

	/**
		���entitiessizeС��256
		ͨ������λ������ȡentityID
		����ֱ��ȡID
	*/
	ENTITY_ID readEntityIDFromStream(MemoryStream& s);

	/**
		��mailbox�����Ի�ȡһ��channel��ʵ��
	*/
	virtual Mercury::Channel* findChannelByMailbox(EntityMailbox& mailbox);

	/** ����ӿ�
		�ͻ��������˵�һ�ν�������, ����˷���
	*/
	virtual void onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo,
		const std::string& scriptVerInfo, const std::string& protocolMD5, 
		const std::string& entityDefMD5, COMPONENT_TYPE componentType);

	virtual void onHelloCB(Mercury::Channel* pChannel, MemoryStream& s);

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
	virtual void onReLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

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
	void onRemoteMethodCall_(ENTITY_ID eid, MemoryStream& s);

	/** ����ӿ�
	   ���߳�������
	*/
	virtual void onKicked(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** ����ӿ�
		����������entity����
	*/
	virtual void onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdatePropertysOptimized(Mercury::Channel* pChannel, MemoryStream& s);
	void onUpdatePropertys_(ENTITY_ID eid, MemoryStream& s);

	/** ����ӿ�
		������ǿ������entity��λ���볯��
	*/
	virtual void onSetEntityPosAndDir(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		����������avatar����λ��
	*/
	virtual void onUpdateBasePos(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onUpdateBasePosXZ(Mercury::Channel* pChannel, MemoryStream& s);

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
	
	void _updateVolatileData(ENTITY_ID entityID, float x, float y, float z, float roll, 
		float pitch, float yaw, int8 isOnGound);

	/** 
		������ҵ������ 
	*/
	virtual void updatePlayerToServer();

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
		���յ�ClientMessages(ͨ����web�ȲŻ�Ӧ�õ�)
	*/
	virtual void onImportClientMessages(Mercury::Channel* pChannel, MemoryStream& s){}

	/** ����ӿ�
		���յ�entitydef(ͨ����web�ȲŻ�Ӧ�õ�)
	*/
	virtual void onImportClientEntityDef(Mercury::Channel* pChannel, MemoryStream& s){}
	
	/** ����ӿ�
		��������������(ͨ����web�ȲŻ�Ӧ�õ�)
	*/
	virtual void onImportServerErrorsDescr(Mercury::Channel* pChannel, MemoryStream& s){}

	/** ����ӿ�
		�����˺��������󷵻�
	*/
	virtual void onReqAccountResetPasswordCB(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** ����ӿ�
		��������䷵��
	*/
	virtual void onReqAccountBindEmailCB(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** ����ӿ�
		�����޸����뷵��
	*/
	virtual void onReqAccountNewPasswordCB(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode){}

	/** 
		���playerʵ��
	*/
	client::Entity* pPlayer();
	void setPlayerPosition(float x, float y, float z){ entityPos_ = Position3D(x, y, z); }
	void setPlayerDirection(float roll, float pitch, float yaw){ entityDir_ = Direction3D(roll, pitch, yaw); }

	void setTargetID(ENTITY_ID id){ 
		targetID_ = id; 
		onTargetChanged();
	}
	ENTITY_ID getTargetID()const{ return targetID_; }
	virtual void onTargetChanged(){}

	ENTITY_ID getAoiEntityID(ENTITY_ID id);
	ENTITY_ID getAoiEntityIDFromStream(MemoryStream& s);
	ENTITY_ID getAoiEntityIDByAliasID(uint8 id);

	/** 
		space��ز����ӿ�
		����������ĳ��space�ļ���ӳ��
	*/
	void addSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath);
	virtual void onAddSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath){}
	virtual void onLoadedSpaceGeometryMapping(SPACE_ID spaceID){
		isLoadedGeometry_ = true;
	}

	const std::string& getGeometryPath();
	
	void initSpaceData(Mercury::Channel* pChannel, MemoryStream& s);
	void setSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value);
	void delSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key);
	bool hasSpaceData(const std::string& key);
	const std::string& getSpaceData(const std::string& key);
	static PyObject* __py_GetSpaceData(PyObject* self, PyObject* args);
	void clearSpace(bool isAll);

	Timers & timers() { return timers_; }
	void handleTimers();

	ScriptCallbacks & scriptCallbacks() { return scriptCallbacks_; }

	void locktime(uint64 t){ locktime_ = t; }
	uint64 locktime()const{ return locktime_; }

	virtual void onServerClosed();

	uint64 rndUUID()const{ return rndUUID_; }
protected:				
	int32													appID_;

	// ���������ͨ��
	Mercury::Channel*										pServerChannel_;

	// �洢���е�entity������
	Entities<client::Entity>*								pEntities_;	
	std::vector<ENTITY_ID>									pEntityIDAliasIDList_;

	PY_CALLBACKMGR											pyCallbackMgr_;

	ENTITY_ID												entityID_;
	SPACE_ID												spaceID_;

	Position3D												entityPos_;
	Direction3D												entityDir_;

	DBID													dbid_;

	std::string												ip_;
	uint16													port_;

	std::string												gatewayIP_;
	uint16													gateWayPort_;

	uint64													lastSentActiveTickTime_;
	uint64													lastSentUpdateDataTime_;

	bool													connectedGateway_;
	bool													canReset_;

	std::string												name_;
	std::string												password_;
	std::string												extradatas_;

	CLIENT_CTYPE											typeClient_;

	typedef std::map<ENTITY_ID, KBEShared_ptr<MemoryStream> > BUFFEREDMESSAGE;
	BUFFEREDMESSAGE											bufferedCreateEntityMessage_;

	EventHandler											eventHandler_;

	Mercury::NetworkInterface&								ninterface_;

	// ��ǰ�ͻ�����ѡ���Ŀ��
	ENTITY_ID												targetID_;

	// �Ƿ���ع���������
	bool													isLoadedGeometry_;

	SPACE_DATA												spacedatas_;

	Timers													timers_;
	ScriptCallbacks											scriptCallbacks_;

	uint64													locktime_;
	
	// �����ص�½����ʱ��key
	uint64													rndUUID_; 
};



}
#endif
