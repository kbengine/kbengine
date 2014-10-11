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

#ifndef KBE_ENGINE_COMPONENT_MGR_HPP
#define KBE_ENGINE_COMPONENT_MGR_HPP
	
// common include
//#define NDEBUG
#include "cstdkbe/timer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "thread/threadmutex.hpp"
#include "thread/threadguard.hpp"
#include "server/common.hpp"

// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

namespace Mercury
{
class Channel;
class Address;
class NetworkInterface;
}

// ComponentInfos.flags��־
#define COMPONENT_FLAG_NORMAL 0x00000000
#define COMPONENT_FLAG_SHUTTINGDOWN 0x00000001

class Components : public Singleton<Components>
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
			pChannel = NULL;
			shutdownState = 0;
			mem = cpu = 0.f;
			usedmem = 0;
			extradata = extradata1 = extradata2 = 0;
			pid = 0;
			externalAddressEx[0] = '\0';
			logTime = timestamp();
		}

		KBEShared_ptr<Mercury::Address > pIntAddr, pExtAddr;	// �ڲ����ⲿ��ַ
		char externalAddressEx[MAX_NAME + 1];					// ǿ�Ʊ�¶���ⲿ�Ĺ�����ַ, ��������е�externalAddressEx

		int32 uid;
		COMPONENT_ID cid;
		COMPONENT_ORDER groupOrderid, globalOrderid;
		char username[MAX_NAME + 1];
		Mercury::Channel* pChannel;
		COMPONENT_TYPE componentType;
		uint32 flags;
		int8 shutdownState;
		float cpu;
		float mem;
		uint32 usedmem;
		uint64 extradata, extradata1, extradata2, extradata3;
		uint32 pid;
		uint64 logTime;
	};

	typedef std::vector<ComponentInfos> COMPONENTS;

	/** ������ɾ��handler */
	class ComponentsNotificationHandler
	{
	public:
		virtual ~ComponentsNotificationHandler() {};
		virtual void onAddComponent(const Components::ComponentInfos*) = 0;
		virtual void onRemoveComponent(const Components::ComponentInfos*) = 0;
	};
public:
	Components();
	~Components();

	INLINE void pNetworkInterface(Mercury::NetworkInterface * networkInterface)
	{ 
		KBE_ASSERT(networkInterface != NULL); 
		_pNetworkInterface = networkInterface; 
	}

	INLINE Mercury::NetworkInterface* pNetworkInterface()
	{ 
		return _pNetworkInterface;
	}

	void addComponent(int32 uid, const char* username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, int8 globalorderid, int8 grouporderid,
		uint32 intaddr, uint16 intport, 
		uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
		float cpu, float mem, uint32 usedmem, uint64 extradata, uint64 extradata1, uint64 extradata2, uint64 extradata3,
		Mercury::Channel* pChannel = NULL);

	void delComponent(int32 uid, COMPONENT_TYPE componentType, 
		COMPONENT_ID componentID, bool ignoreComponentID = false, bool shouldShowLog = true);

	void removeComponentFromChannel(Mercury::Channel * pChannel);

	void clear(int32 uid = -1, bool shouldShowLog = true);

	Components::COMPONENTS& getComponents(COMPONENT_TYPE componentType);

	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(Mercury::Channel * pChannel);
	//const Components::ComponentInfos findComponent(COMPONENT_TYPE componentType, int32 uid);

	int connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);

	typedef std::map<int32/*uid*/, int32/*lastorder*/> ORDER_LOG;
	ORDER_LOG& getGlobalOrderLog(){ return _globalOrderLog; }
	ORDER_LOG& getBaseappGroupOrderLog(){ return _baseappGrouplOrderLog; }
	ORDER_LOG& getCellappGroupOrderLog(){ return _cellappGrouplOrderLog; }
	ORDER_LOG& getLoginappGroupOrderLog(){ return _loginappGrouplOrderLog; }
	
	/** 
		������е������ ��ֹ���ظ���uuid�� ��ʱӦ�ñ���.
	*/
	bool checkComponents(int32 uid, COMPONENT_ID componentID);

	void pHandler(ComponentsNotificationHandler* ph){ _pHandler = ph; };

	/** 
		���ĳ������Ƿ���Ч.
	*/
	bool checkComponentUsable(const Components::ComponentInfos* info);

	Components::ComponentInfos* getBaseappmgr();
	Components::ComponentInfos* getCellappmgr();
	Components::ComponentInfos* getDbmgr();
	Components::ComponentInfos* getMessagelog();
	Components::ComponentInfos* getBillings();

	Mercury::Channel* getBaseappmgrChannel();
	Mercury::Channel* getCellappmgrChannel();
	Mercury::Channel* getDbmgrChannel();
	Mercury::Channel* getMessagelogChannel();

private:
	COMPONENTS								_baseapps;
	COMPONENTS								_cellapps;
	COMPONENTS								_dbmgrs;
	COMPONENTS								_loginapps;
	COMPONENTS								_cellappmgrs;
	COMPONENTS								_baseappmgrs;
	COMPONENTS								_machines;
	COMPONENTS								_messagelogs;
	COMPONENTS								_billings;
	COMPONENTS								_bots;
	COMPONENTS								_consoles;

	Mercury::NetworkInterface*				_pNetworkInterface;
	
	// �����ȫ����������log����(��ͬ�����Ϊһ�飬 �磺����baseappΪһ����)��������log
	// ע��:��;��������app�������log����ȥ��������, ��ʹ����ͼ����Ҳû�б�Ҫ�����ƥ�䡣
	ORDER_LOG								_globalOrderLog;
	ORDER_LOG								_baseappGrouplOrderLog;
	ORDER_LOG								_cellappGrouplOrderLog;
	ORDER_LOG								_loginappGrouplOrderLog;

	ComponentsNotificationHandler*			_pHandler;
};

}

#endif // KBE_ENGINE_COMPONENT_MGR_HPP
