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


#ifndef KBE_SERVER_APP_HPP
#define KBE_SERVER_APP_HPP

// common include
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include <stdarg.h> 
#include "helper/debug_helper.hpp"
#include "helper/watcher.hpp"
#include "helper/profile.hpp"
#include "helper/profiler.hpp"
#include "helper/profile_handler.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "server/common.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "server/signal_handler.hpp"
#include "server/shutdown_handler.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/singleton.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "thread/threadpool.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

namespace Mercury
{

class Channel;
}

class Shutdowner;
class ComponentActiveReportHandler;

class ServerApp : 
	public SignalHandler, 
	public TimerHandler, 
	public ShutdownHandler,
	public Mercury::ChannelTimeOutHandler,
	public Mercury::ChannelDeregisterHandler,
	public Components::ComponentsNotificationHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_SERVERAPP_MAX
	};
public:
	ServerApp(Mercury::EventDispatcher& dispatcher, 
			Mercury::NetworkInterface& ninterface, 
			COMPONENT_TYPE componentType,
			COMPONENT_ID componentID);

	~ServerApp();

	virtual bool initialize();
	virtual bool initializeBegin(){return true;};
	virtual bool inInitialize(){ return true; }
	virtual bool initializeEnd(){return true;};
	virtual void finalise();
	virtual bool run();
	
	virtual bool initThreadPool();

	bool installSingnals();

	virtual bool initializeWatcher();

	virtual bool loadConfig();
	const char* name(){return COMPONENT_NAME_EX(componentType_);}
	
	virtual void handleTimeout(TimerHandle, void * pUser);

	GAME_TIME time() const { return g_kbetime; }
	Timers & timers() { return timers_; }
	double gameTimeInSeconds() const;
	void handleTimers();

	thread::ThreadPool& threadPool(){ return threadPool_; }

	Mercury::EventDispatcher & mainDispatcher()				{ return mainDispatcher_; }
	Mercury::NetworkInterface & networkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID()const	{ return componentID_; }
	COMPONENT_TYPE componentType()const	{ return componentType_; }
		
	virtual void onSignalled(int sigNum);
	virtual void onChannelTimeOut(Mercury::Channel * pChannel);
	virtual void onChannelDeregister(Mercury::Channel * pChannel);
	virtual void onAddComponent(const Components::ComponentInfos* pInfos);
	virtual void onRemoveComponent(const Components::ComponentInfos* pInfos);

	virtual void onShutdownBegin();
	virtual void onShutdown(bool first);
	virtual void onShutdownEnd();

	/** ����ӿ�
		����鿴watcher
	*/
	void queryWatcher(Mercury::Channel* pChannel, MemoryStream& s);

	void shutDown(float shutdowntime = -FLT_MAX);

	int32 globalOrder()const{ return startGlobalOrder_; }
	int32 groupOrder()const{ return startGroupOrder_; }

	/** ����ӿ�
		ע��һ���¼����baseapp����cellapp����dbmgr
		ͨ����һ���µ�app�������ˣ� ����Ҫ��ĳЩ���ע���Լ���
	*/
	virtual void onRegisterNewApp(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);

	/** ����ӿ�
		ĳ��app��app��֪���ڻ״̬��
	*/
	void onAppActiveTick(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	
	/** ����ӿ�
		����Ͽ�������������
	*/
	virtual void reqClose(Mercury::Channel* pChannel);

	/** ����ӿ�
		ĳ��app����鿴��app
	*/
	virtual void lookApp(Mercury::Channel* pChannel);

	/** ����ӿ�
		����رշ�����
	*/
	virtual void reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		ĳ��app����鿴��app����״̬�� ͨ����console����鿴
	*/
	virtual void queryLoad(Mercury::Channel* pChannel);

	/** ����ӿ�
		����رշ�����
	*/
	void reqKillServer(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		�ͻ��������˵�һ�ν�������, �ͻ��˷����Լ��İ汾����ͨѶ��Կ����Ϣ
		������ˣ� ����˷����Ƿ����ֳɹ�
	*/
	virtual void hello(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onHello(Mercury::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	// ����汾��ƥ��
	virtual void onVersionNotMatch(Mercury::Channel* pChannel);

	// ����ű���汾��ƥ��
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel);

	/** ����ӿ�
		console����ʼprofile
	*/
	void startProfile(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	virtual void startProfile_(Mercury::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen);
protected:
	COMPONENT_TYPE											componentType_;
	COMPONENT_ID											componentID_;									// �������ID

	Mercury::EventDispatcher& 								mainDispatcher_;	
	Mercury::NetworkInterface&								networkInterface_;
	
	Timers													timers_;

	// app����˳�� globalΪȫ��(��dbmgr��cellapp��˳��)����˳�� 
	// groupΪ������˳��(��:����baseappΪһ��)
	int32													startGlobalOrder_;
	int32													startGroupOrder_;

	Shutdowner*												pShutdowner_;
	ComponentActiveReportHandler*							pActiveTimerHandle_;

	// �̳߳�
	thread::ThreadPool										threadPool_;	
};

}

#endif // KBE_SERVER_APP_HPP
