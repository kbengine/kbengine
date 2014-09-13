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



#ifndef KBE_BASE_HPP
#define KBE_BASE_HPP
	
// common include	
#include "profile.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "pyscript/math.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/datatypes.hpp"	
#include "entitydef/entitydef.hpp"	
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entity_macro.hpp"	
#include "server/script_timers.hpp"		

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

class EntityMailbox;
class BaseMessagesForwardHandler;

namespace Mercury
{
class Channel;
}


class Base : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	BASE_SCRIPT_HREADER(Base, ScriptObject)	
	ENTITY_HEADER(Base)
public:
	Base(ENTITY_ID id, const ScriptDefModule* scriptModule, 
		PyTypeObject* pyType = getScriptType(), bool isInitialised = true);
	~Base();

	/** 
		是否存储数据库 
	*/
	INLINE bool hasDB()const;
	INLINE void hasDB(bool has);

	/** 
		数据库关联ID
	*/
	INLINE DBID dbid()const;
	INLINE void dbid(DBID id);
	DECLARE_PY_GET_MOTHOD(pyGetDBID);

	/** 
		销毁cell部分的实体 
	*/
	bool destroyCellEntity(void);

	DECLARE_PY_MOTHOD_ARG0(pyDestroyCellEntity);
	
	/** 
		脚本请求销毁base实体 
	*/
	DECLARE_PY_MOTHOD_ARG0(pyDestroyBase);
	
	/** 
		脚本获取mailbox 
	*/
	DECLARE_PY_GET_MOTHOD(pyGetCellMailbox);

	EntityMailbox* cellMailbox(void)const;

	void cellMailbox(EntityMailbox* mailbox);
	
	/** 
		脚本获取mailbox 
	*/
	DECLARE_PY_GET_MOTHOD(pyGetClientMailbox);

	EntityMailbox* clientMailbox()const;

	void clientMailbox(EntityMailbox* mailbox);

	/**
		是否创建过space
	*/
	INLINE bool isCreatedSpace();

	/** 
		cellData部分 
	*/
	bool installCellDataAttr(PyObject* dictData = NULL, bool installpy = true);

	void createCellData(void);

	void destroyCellData(void);

	void addPersistentsDataToStream(uint32 flags, MemoryStream* s);

	PyObject* createCellDataDict(uint32 flags);

	INLINE PyObject* getCellData(void)const;
	
	INLINE bool creatingCell(void)const;

	/**
		请求cell部分将entity的celldata更新一份过来
	*/
	void reqBackupCellData();
	
	/** 
		写备份信息到流
	*/
	void writeBackupData(MemoryStream* s);
	void onBackup();

	/** 
		写存档信息到流
	*/
	void writeArchiveData(MemoryStream* s);

	/** 
		将要保存到数据库之前的通知 
	*/
	void onWriteToDB();
	void onCellWriteToDBCompleted(CALLBACK_ID callbackID);
	void onWriteToDBCallback(ENTITY_ID eid, DBID entityDBID, 
		CALLBACK_ID callbackID, bool success);

	/** 网络接口
		entity第一次写数据库由dbmgr返回的dbid
	*/
	void onGetDBID(Mercury::Channel* pChannel, DBID dbid);

	/** 
		创建cell失败回调 
	*/
	void onCreateCellFailure(void);

	/** 
		创建cell成功回调 
	*/
	void onGetCell(Mercury::Channel* pChannel, COMPONENT_ID componentID);

	/** 
		丢失cell了的通知 
	*/
	void onLoseCell(Mercury::Channel* pChannel, MemoryStream& s);

	/** 
		当cellapp意外终止后， baseapp如果能找到合适的cellapp则将其恢复后
		会调用此方法
	*/
	void onRestore();

	/** 
		备份cell数据
	*/
	void onBackupCellData(Mercury::Channel* pChannel, MemoryStream& s);

	/** 
		客户端丢失 
	*/
	void onClientDeath();

	/** 网络接口
		远程呼叫本entity的方法 
	*/
	void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);

	/** 
		销毁这个entity 
	*/
	void onDestroy(bool callScript);

	/**
		销毁base内部通知
	*/
	void onDestroyEntity(bool deleteFromDB, bool writeToDB);

	/** 
		为一个baseEntity在指定的cell上创建一个cellEntity 
	*/
	DECLARE_PY_MOTHOD_ARG1(createCellEntity, PyObject_ptr);
	
	/** 
		为一个baseEntity在指定的cell上还原一个cellEntity 
	*/
	void restoreCell(EntityMailboxAbstract* cellMailbox);
	INLINE bool inRestore();

	/** 
		创建一个cellEntity在一个新的space上 
	*/
	DECLARE_PY_MOTHOD_ARG1(createInNewSpace, PyObject_ptr);

	/** 网络接口
		客户端直接发送消息给cell实体
	*/
	void forwardEntityMessageToCellappFromClient(Mercury::Channel* pChannel, MemoryStream& s);
	
	/**
		发送消息到cellapp上
	*/
	void sendToCellapp(Mercury::Bundle* pBundle);
	void sendToCellapp(Mercury::Channel* pChannel, Mercury::Bundle* pBundle);

	/** 
		传送
	*/
	DECLARE_PY_MOTHOD_ARG1(pyTeleport, PyObject_ptr);

	/**
		传送回调
	*/
	void onTeleportCB(Mercury::Channel* pChannel, SPACE_ID spaceID, bool fromCellTeleport);  
	void onTeleportFailure();  
	void onTeleportSuccess(SPACE_ID spaceID);

	/** 网络接口
		某个entity请求teleport到这个entity的space上。
	*/
	void reqTeleportOther(Mercury::Channel* pChannel, ENTITY_ID reqTeleportEntityID, 
		COMPONENT_ID reqTeleportEntityCellAppID, COMPONENT_ID reqTeleportEntityBaseAppID);

	/** 网络接口
		entity请求迁移到另一个cellapp上的过程开始和结束。
	*/
	void onMigrationCellappStart(Mercury::Channel* pChannel, COMPONENT_ID cellappID);
	void onMigrationCellappEnd(Mercury::Channel* pChannel, COMPONENT_ID cellappID);

	/**
		设置获取是否自动存档
	*/
	INLINE int8 shouldAutoArchive()const;
	INLINE void shouldAutoArchive(int8 v);
	DECLARE_PY_GETSET_MOTHOD(pyGetShouldAutoArchive, pySetShouldAutoArchive);

	/**
		设置获取是否自动备份
	*/
	INLINE int8 shouldAutoBackup()const;
	INLINE void shouldAutoBackup(int8 v);
	DECLARE_PY_GETSET_MOTHOD(pyGetShouldAutoBackup, pySetShouldAutoBackup);

	/**
		cellapp宕了
	*/
	void onCellAppDeath();

	/** 
		转发消息完成 
	*/
	void onBufferedForwardToCellappMessagesOver();

protected:
	/** 
		定义属性数据被改变了 
	*/
	void onDefDataChanged(const PropertyDescription* propertyDescription, 
			PyObject* pyData);

	/**
		从db擦除在线log
	*/
	void eraseEntityLog();

protected:
	// 这个entity的客户端mailbox cellapp mailbox
	EntityMailbox*							clientMailbox_;			
	EntityMailbox*							cellMailbox_;

	// entity创建后，在cell部分未创建时，将一些cell属性数据保存在这里
	PyObject*								cellDataDict_;			

	// 是否是存储到数据库中的entity
	bool									hasDB_;					
	DBID									DBID_;

	// 是否正在获取celldata中
	bool									isGetingCellData_;

	// 是否正在存档中
	bool									isArchiveing_;

	// 是否进行自动存档 <= 0为false, 1为true, KBE_NEXT_ONLY为执行一次后自动为false
	int8									shouldAutoArchive_;
	
	// 是否进行自动备份 <= 0为false, 1为true, KBE_NEXT_ONLY为执行一次后自动为false
	int8									shouldAutoBackup_;

	// 是否正在创建cell中
	bool									creatingCell_;

	// 是否已经创建了一个space
	bool									createdSpace_;

	// 是否正在恢复
	bool									inRestore_;

	// 在一些状态下(传送过程中)，发往cellapp的数据包需要被缓存, 合适的状态需要继续转发
	BaseMessagesForwardHandler*				pBufferedSendToCellappMessages_;
};

}


#ifdef CODE_INLINE
#include "base.ipp"
#endif

#endif // KBE_BASE_HPP
