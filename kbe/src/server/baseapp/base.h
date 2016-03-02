/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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



#ifndef KBE_BASE_H
#define KBE_BASE_H
	
#include "profile.h"
#include "common/common.h"
#include "helper/debug_helper.h"
#include "pyscript/math.h"
#include "pyscript/scriptobject.h"
#include "entitydef/datatypes.h"	
#include "entitydef/entitydef.h"	
#include "entitydef/scriptdef_module.h"
#include "entitydef/entity_macro.h"	
#include "server/script_timers.h"		
	
namespace KBEngine{

class EntityMailbox;
class BaseMessagesForwardHandler;

namespace Network
{
class Channel;
}


class Base : public script::ScriptObject
{
	/** ���໯ ��һЩpy�������������� */
	BASE_SCRIPT_HREADER(Base, ScriptObject)	
	ENTITY_HEADER(Base)
public:
	Base(ENTITY_ID id, const ScriptDefModule* pScriptModule, 
		PyTypeObject* pyType = getScriptType(), bool isInitialised = true);
	~Base();

	/** 
		�Ƿ�洢���ݿ� 
	*/
	INLINE bool hasDB() const;
	INLINE void hasDB(bool has);

	/** 
		���ݿ����ID
	*/
	INLINE DBID dbid() const;
	INLINE void dbid(uint16 dbInterfaceIndex, DBID id);
	DECLARE_PY_GET_MOTHOD(pyGetDBID);

	/** 
		����cell���ֵ�ʵ�� 
	*/
	bool destroyCellEntity(void);

	DECLARE_PY_MOTHOD_ARG0(pyDestroyCellEntity);
	
	/** 
		�ű���������baseʵ�� 
	*/
	DECLARE_PY_MOTHOD_ARG0(pyDestroyBase);
	
	/** 
		�ű���ȡmailbox 
	*/
	DECLARE_PY_GET_MOTHOD(pyGetCellMailbox);

	EntityMailbox* cellMailbox(void) const;

	void cellMailbox(EntityMailbox* mailbox);
	
	/** 
		�ű���ȡmailbox 
	*/
	DECLARE_PY_GET_MOTHOD(pyGetClientMailbox);

	EntityMailbox* clientMailbox() const;

	void clientMailbox(EntityMailbox* mailbox);

	/**
		�Ƿ񴴽���space
	*/
	INLINE bool isCreatedSpace();

	/** 
		cellData���� 
	*/
	bool installCellDataAttr(PyObject* dictData = NULL, bool installpy = true);

	void createCellData(void);

	void destroyCellData(void);

	void addPersistentsDataToStream(uint32 flags, MemoryStream* s);

	PyObject* createCellDataDict(uint32 flags);

	INLINE PyObject* getCellData(void) const;
	
	INLINE bool creatingCell(void) const;

	/**
		����cell���ֽ�entity��celldata����һ�ݹ���
	*/
	void reqBackupCellData();
	
	/** 
		д������Ϣ����
	*/
	void writeBackupData(MemoryStream* s);
	void onBackup();

	/** 
		д�浵��Ϣ����
	*/
	void writeArchiveData(MemoryStream* s);

	/** 
		��Ҫ���浽���ݿ�֮ǰ��֪ͨ 
	*/
	void onWriteToDB();
	void onCellWriteToDBCompleted(CALLBACK_ID callbackID, int8 shouldAutoLoad, int dbInterfaceIndex);
	void onWriteToDBCallback(ENTITY_ID eid, DBID entityDBID, uint16 dbInterfaceIndex,
		CALLBACK_ID callbackID, int8 shouldAutoLoad, bool success);

	/** ����ӿ�
		entity��һ��д���ݿ���dbmgr���ص�dbid
	*/
	void onGetDBID(Network::Channel* pChannel, DBID dbid);

	/** 
		����cellʧ�ܻص� 
	*/
	void onCreateCellFailure(void);

	/** 
		����cell�ɹ��ص� 
	*/
	void onGetCell(Network::Channel* pChannel, COMPONENT_ID componentID);

	/** 
		��ʧcell�˵�֪ͨ 
	*/
	void onLoseCell(Network::Channel* pChannel, MemoryStream& s);

	/** 
		��cellapp������ֹ�� baseapp������ҵ����ʵ�cellapp����ָ���
		����ô˷���
	*/
	void onRestore();

	/** 
		����cell����
	*/
	void onBackupCellData(Network::Channel* pChannel, MemoryStream& s);

	/** 
		�ͻ��˶�ʧ 
	*/
	void onClientDeath();

	/** ����ӿ�
		Զ�̺��б�entity�ķ��� 
	*/
	void onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s);

	/** 
		�������entity 
	*/
	void onDestroy(bool callScript);

	/**
		����base�ڲ�֪ͨ
	*/
	void onDestroyEntity(bool deleteFromDB, bool writeToDB);

	/** 
		Ϊһ��baseEntity��ָ����cell�ϴ���һ��cellEntity 
	*/
	DECLARE_PY_MOTHOD_ARG1(createCellEntity, PyObject_ptr);
	
	/** 
		Ϊһ��baseEntity��ָ����cell�ϻ�ԭһ��cellEntity 
	*/
	void restoreCell(EntityMailboxAbstract* cellMailbox);
	INLINE bool inRestore();

	/** 
		����һ��cellEntity��һ���µ�space�� 
	*/
	DECLARE_PY_MOTHOD_ARG1(createInNewSpace, PyObject_ptr);

	/** ����ӿ�
		�ͻ���ֱ�ӷ�����Ϣ��cellʵ��
	*/
	void forwardEntityMessageToCellappFromClient(Network::Channel* pChannel, MemoryStream& s);
	
	/**
		������Ϣ��cellapp��
	*/
	void sendToCellapp(Network::Bundle* pBundle);
	void sendToCellapp(Network::Channel* pChannel, Network::Bundle* pBundle);

	/** 
		����
	*/
	DECLARE_PY_MOTHOD_ARG1(pyTeleport, PyObject_ptr);

	/**
		���ͻص�
	*/
	void onTeleportCB(Network::Channel* pChannel, SPACE_ID spaceID, bool fromCellTeleport);  
	void onTeleportFailure();  
	void onTeleportSuccess(SPACE_ID spaceID);

	/** ����ӿ�
		ĳ��entity����teleport�����entity��space�ϡ�
	*/
	void reqTeleportOther(Network::Channel* pChannel, ENTITY_ID reqTeleportEntityID, 
		COMPONENT_ID reqTeleportEntityCellAppID, COMPONENT_ID reqTeleportEntityBaseAppID);

	/** ����ӿ�
		entity����Ǩ�Ƶ���һ��cellapp�ϵĹ��̿�ʼ�ͽ�����
	*/
	void onMigrationCellappStart(Network::Channel* pChannel, COMPONENT_ID cellappID);
	void onMigrationCellappEnd(Network::Channel* pChannel, COMPONENT_ID cellappID);

	/**
		���û�ȡ�Ƿ��Զ��浵
	*/
	INLINE int8 shouldAutoArchive() const;
	INLINE void shouldAutoArchive(int8 v);
	DECLARE_PY_GETSET_MOTHOD(pyGetShouldAutoArchive, pySetShouldAutoArchive);

	/**
		���û�ȡ�Ƿ��Զ�����
	*/
	INLINE int8 shouldAutoBackup() const;
	INLINE void shouldAutoBackup(int8 v);
	DECLARE_PY_GETSET_MOTHOD(pyGetShouldAutoBackup, pySetShouldAutoBackup);

	/**
		cellapp���
	*/
	void onCellAppDeath();

	/** 
		ת����Ϣ��� 
	*/
	void onBufferedForwardToCellappMessagesOver();

	/** 
		����ʵ��־û������Ƿ����࣬���˻��Զ��浵 
	*/
	INLINE void setDirty(bool dirty = true);
	INLINE bool isDirty() const;
	
	INLINE uint16 dbInterfaceIndex() const;

protected:
	/** 
		�����������ݱ��ı��� 
	*/
	void onDefDataChanged(const PropertyDescription* propertyDescription, 
			PyObject* pyData);

	/**
		��db��������log
	*/
	void eraseEntityLog();

protected:
	// ���entity�Ŀͻ���mailbox cellapp mailbox
	EntityMailbox*							clientMailbox_;			
	EntityMailbox*							cellMailbox_;

	// entity��������cell����δ����ʱ����һЩcell�������ݱ���������
	PyObject*								cellDataDict_;			

	// �Ƿ��Ǵ洢�����ݿ��е�entity
	bool									hasDB_;					
	DBID									DBID_;

	// �Ƿ����ڻ�ȡcelldata��
	bool									isGetingCellData_;

	// �Ƿ����ڴ浵��
	bool									isArchiveing_;

	// �Ƿ�����Զ��浵 <= 0Ϊfalse, 1Ϊtrue, KBE_NEXT_ONLYΪִ��һ�κ��Զ�Ϊfalse
	int8									shouldAutoArchive_;
	
	// �Ƿ�����Զ����� <= 0Ϊfalse, 1Ϊtrue, KBE_NEXT_ONLYΪִ��һ�κ��Զ�Ϊfalse
	int8									shouldAutoBackup_;

	// �Ƿ����ڴ���cell��
	bool									creatingCell_;

	// �Ƿ��Ѿ�������һ��space
	bool									createdSpace_;

	// �Ƿ����ڻָ�
	bool									inRestore_;

	// ��һЩ״̬��(���͹�����)������cellapp�����ݰ���Ҫ������, ���ʵ�״̬��Ҫ����ת��
	BaseMessagesForwardHandler*				pBufferedSendToCellappMessages_;
	
	// ��Ҫ�־û��������Ƿ���࣬���û�б��಻��Ҫ�־û�
	bool									isDirty_;

	// ������ʵ���Ѿ�д�����ݿ⣬��ô������Ծ��Ƕ�Ӧ�����ݿ�ӿڵ�����
	uint16									dbInterfaceIndex_;
};

}


#ifdef CODE_INLINE
#include "base.inl"
#endif

#endif // KBE_BASE_H
