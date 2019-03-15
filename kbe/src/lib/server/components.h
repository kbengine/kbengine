// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ENGINE_COMPONENT_MGR_H
#define KBE_ENGINE_COMPONENT_MGR_H
	
#include "common/timer.h"
#include "common/tasks.h"
#include "common/common.h"
#include "common/singleton.h"
#include "thread/threadmutex.h"
#include "thread/threadguard.h"
#include "server/common.h"
	
namespace KBEngine{

namespace Network
{
class Channel;
class Address;
class NetworkInterface;
}

// ComponentInfos.flags标志
#define COMPONENT_FLAG_NORMAL 0x00000000
#define COMPONENT_FLAG_SHUTTINGDOWN 0x00000001

class Components : public Task, public Singleton<Components>
{
public:
	static int32 ANY_UID; 

	struct ComponentInfos
	{
		ComponentInfos()
		{
			uid = 0;
			flags = COMPONENT_FLAG_NORMAL;
			cid = 0;

			groupOrderid = 0;
			globalOrderid = 0;
			gus = 0;

			pChannel = NULL;
			state = COMPONENT_STATE_INIT;
			mem = cpu = 0.f;
			usedmem = 0;

			extradata = extradata1 = extradata2 = 0;

			pid = 0;
			externalAddressEx[0] = '\0';
			logTime = timestamp();
			appFlags = APP_FLAGS_NONE;
		}

		KBEShared_ptr<Network::Address> pIntAddr, pExtAddr;		// 内部和外部地址
		char externalAddressEx[MAX_NAME + 1];					// 强制暴露给外部的公网地址, 详见配置中的externalAddressEx

		int32 uid;
		COMPONENT_ID cid;
		COMPONENT_ORDER groupOrderid, globalOrderid;
		COMPONENT_GUS gus;
		char username[MAX_NAME + 1];
		Network::Channel* pChannel;
		COMPONENT_TYPE componentType;
		uint32 flags;

		// 进程状态
		COMPONENT_STATE state;

		float cpu;
		float mem;
		uint32 usedmem;
		uint64 extradata, extradata1, extradata2, extradata3;
		uint32 pid;
		uint64 logTime;
		uint32 appFlags;
	};

	typedef std::vector<ComponentInfos> COMPONENTS;

	/** 组件添加删除handler */
	class ComponentsNotificationHandler
	{
	public:
		virtual ~ComponentsNotificationHandler() {};
		virtual void onAddComponent(const Components::ComponentInfos*) = 0;
		virtual void onRemoveComponent(const Components::ComponentInfos*) = 0;
		virtual void onIdentityillegal(COMPONENT_TYPE componentType, COMPONENT_ID componentID, uint32 pid, const char* pAddr) = 0;
	};

public:
	Components();
	~Components();

	void initialize(Network::NetworkInterface * pNetworkInterface, COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	void finalise();

	INLINE Network::NetworkInterface* pNetworkInterface()
	{ 
		return _pNetworkInterface;
	}

	void addComponent(int32 uid, const char* username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderid, COMPONENT_ORDER grouporderid, COMPONENT_GUS gus,
		uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
		float cpu, float mem, uint32 usedmem, uint64 extradata, uint64 extradata1, uint64 extradata2, uint64 extradata3,
		Network::Channel* pChannel = NULL);

	void delComponent(int32 uid, COMPONENT_TYPE componentType, 
		COMPONENT_ID componentID, bool ignoreComponentID = false, bool shouldShowLog = true);

	void removeComponentByChannel(Network::Channel * pChannel, bool isShutingdown = false);

	void clear(int32 uid = -1, bool shouldShowLog = true);

	Components::COMPONENTS& getComponents(COMPONENT_TYPE componentType);

	/** 
		查找组件
	*/
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(Network::Channel * pChannel);
	Components::ComponentInfos* findComponent(Network::Address* pAddress);

	/** 
		通过进程id寻找本地组件
	*/
	Components::ComponentInfos* findLocalComponent(uint32 pid);

	int connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID, bool printlog = true);

	typedef std::map<int32/*uid*/, COMPONENT_ORDER/*lastorder*/> ORDER_LOG;
	ORDER_LOG& getGlobalOrderLog(){ return _globalOrderLog; }
	ORDER_LOG& getBaseappGroupOrderLog(){ return _baseappGrouplOrderLog; }
	ORDER_LOG& getCellappGroupOrderLog(){ return _cellappGrouplOrderLog; }
	ORDER_LOG& getLoginappGroupOrderLog(){ return _loginappGrouplOrderLog; }
	
	/** 
		检查所有的组件， 防止有重复的uuid， 此时应该报错.
	*/
	bool checkComponents(int32 uid, COMPONENT_ID componentID, uint32 pid);

	/** 
		设置用于接收组件通知的处理器实例
	*/
	void pHandler(ComponentsNotificationHandler* ph){ _pHandler = ph; };

	/** 
		检查某个组件端口是否有效.
	*/
	bool updateComponentInfos(const Components::ComponentInfos* info);

	/** 
		是否是本地组件.
	*/
	bool isLocalComponent(const Components::ComponentInfos* info);

	/** 
		是否本地组件是否在运行中.
	*/
	const Components::ComponentInfos* lookupLocalComponentRunning(uint32 pid);

	Components::ComponentInfos* getBaseappmgr();
	Components::ComponentInfos* getCellappmgr();
	Components::ComponentInfos* getDbmgr();
	Components::ComponentInfos* getLogger();
	Components::ComponentInfos* getInterfaceses();

	Network::Channel* getBaseappmgrChannel();
	Network::Channel* getCellappmgrChannel();
	Network::Channel* getDbmgrChannel();
	Network::Channel* getLoggerChannel();

	/**
		统计某个UID下的所有组件数量
	*/
	size_t getGameSrvComponentsSize(int32 uid);

	/** 
		获取游戏服务端必要组件的注册数量。
	*/
	size_t getGameSrvComponentsSize();

	void componentID(COMPONENT_ID id){ componentID_ = id; }
	COMPONENT_ID componentID() const { return componentID_; }
	void componentType(COMPONENT_TYPE t){ componentType_ = t; }
	COMPONENT_TYPE componentType() const { return componentType_; }
	
	Network::EventDispatcher & dispatcher();

	void onChannelDeregister(Network::Channel * pChannel, bool isShutingdown);

	void extraData1(uint64 v){ extraData1_ = v; }
	void extraData2(uint64 v){ extraData2_ = v; }
	void extraData3(uint64 v){ extraData3_ = v; }
	void extraData4(uint64 v){ extraData4_ = v; }

	bool findLogger();
	
	void broadcastSelf();

private:
	virtual bool process();
	bool findComponents();

	void onFoundAllComponents();

private:
	COMPONENTS								_baseapps;
	COMPONENTS								_cellapps;
	COMPONENTS								_dbmgrs;
	COMPONENTS								_loginapps;
	COMPONENTS								_cellappmgrs;
	COMPONENTS								_baseappmgrs;
	COMPONENTS								_machines;
	COMPONENTS								_loggers;
	COMPONENTS								_interfaceses;
	COMPONENTS								_bots;
	COMPONENTS								_consoles;

	Network::NetworkInterface*				_pNetworkInterface;
	
	// 组件的全局启动次序log和组(相同的组件为一组， 如：所有baseapp为一个组)启动次序log
	// 注意:中途有死掉的app组件这里log并不去做减操作, 从使用意图来看也没有必要做这个匹配。
	ORDER_LOG								_globalOrderLog;
	ORDER_LOG								_baseappGrouplOrderLog;
	ORDER_LOG								_cellappGrouplOrderLog;
	ORDER_LOG								_loginappGrouplOrderLog;

	ComponentsNotificationHandler*			_pHandler;

	// 本组件的类别
	COMPONENT_TYPE							componentType_;

	// 本组件的ID
	COMPONENT_ID							componentID_;	

	uint8									state_;
	int16									findIdx_;
	int8									findComponentTypes_[8];

	uint64									extraData1_;
	uint64									extraData2_;
	uint64									extraData3_;
	uint64									extraData4_;
};

}

#endif // KBE_ENGINE_COMPONENT_MGR_H
