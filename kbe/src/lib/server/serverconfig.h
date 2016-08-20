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

/*
		ServerConfig::getSingleton().loadConfig("../../res/server/KBEngine.xml");
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();													
*/
#ifndef KBE_SERVER_CONFIG_H
#define KBE_SERVER_CONFIG_H

#define __LIB_DLLAPI__	

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <stdarg.h> 
#include "common/singleton.h"
#include "thread/threadmutex.h"
#include "thread/threadguard.h"
#include "xml/xml.h"	

	
namespace KBEngine{
namespace Network
{
class Address;
}

struct Profiles_Config
{
	Profiles_Config():
		open_pyprofile(false),
		open_cprofile(false),
		open_eventprofile(false),
		open_networkprofile(false)
	{
	}

	bool open_pyprofile;
	bool open_cprofile;
	bool open_eventprofile;
	bool open_networkprofile;
};

struct ChannelCommon
{
	float channelInternalTimeout;
	float channelExternalTimeout;
	uint32 extReadBufferSize;
	uint32 extWriteBufferSize;
	uint32 intReadBufferSize;
	uint32 intWriteBufferSize;
	uint32 intReSendInterval;
	uint32 intReSendRetries;
	uint32 extReSendInterval;
	uint32 extReSendRetries;
};

struct EmailServerInfo
{
	std::string smtp_server;
	uint32 smtp_port;
	std::string username;
	std::string password;
	uint8 smtp_auth;
};

struct EmailSendInfo
{
	std::string subject;
	std::string message;
	std::string backlink_success_message, backlink_fail_message, backlink_hello_message;

	uint32 deadline;
};

struct DBInterfaceInfo
{
	DBInterfaceInfo()
	{
		index = 0;
		isPure = false;
		db_numConnections = 5;
		db_passwordEncrypt = true;

		memset(name, 0, sizeof(name));
		memset(db_type, 0, sizeof(db_type));
		memset(db_ip, 0, sizeof(db_ip));
		memset(db_username, 0, sizeof(db_username));
		memset(db_password, 0, sizeof(db_password));
		memset(db_name, 0, sizeof(db_name));
	}

	int index;
	bool isPure;											// �Ƿ�Ϊ�����⣨û�����洴����ʵ���
	char name[MAX_BUF];										// ���ݿ�Ľӿ�����
	char db_type[MAX_BUF];									// ���ݿ�����
	uint32 db_port;											// ���ݿ�Ķ˿�
	char db_ip[MAX_BUF];									// ���ݿ��ip��ַ
	char db_username[MAX_NAME];								// ���ݿ���û���
	char db_password[MAX_BUF * 10];							// ���ݿ������
	bool db_passwordEncrypt;								// db�����Ƿ��Ǽ��ܵ�
	char db_name[MAX_NAME];									// ���ݿ���
	uint16 db_numConnections;								// ���ݿ��������
	std::string db_unicodeString_characterSet;				// �������ݿ��ַ���
	std::string db_unicodeString_collation;
};

// ���������Ϣ�ṹ��
typedef struct EngineComponentInfo
{
	EngineComponentInfo()
	{
		tcp_SOMAXCONN = 5;
		notFoundAccountAutoCreate = false;
		account_registration_enable = false;
		use_coordinate_system = true;
		account_type = 3;
		debugDBMgr = false;

		externalAddress[0] = '\0';

		isOnInitCallPropertysSetMethods = true;
	}

	~EngineComponentInfo()
	{
	}

	uint32 port;											// ��������к�����Ķ˿�
	char ip[MAX_BUF];										// �����������ip��ַ

	std::vector< std::string > machine_addresses;			// �����и��������е�machine�ĵ�ַ
	
	char entryScriptFile[MAX_NAME];							// �������ڽű��ļ�
	char dbAccountEntityScriptType[MAX_NAME];				// ���ݿ��ʺŽű����
	float defaultAoIRadius;									// ������cellapp�ڵ��е�player��aoi�뾶��С
	float defaultAoIHysteresisArea;							// ������cellapp�ڵ��е�player��aoi���ͺ�Χ
	uint16 witness_timeout;									// �۲���Ĭ�ϳ�ʱʱ��(��)
	const Network::Address* externalAddr;					// �ⲿ��ַ
	const Network::Address* internalAddr;					// �ڲ���ַ
	COMPONENT_ID componentID;

	float ghostDistance;									// ghost�������
	uint16 ghostingMaxPerCheck;								// ÿ����ghost����
	uint16 ghostUpdateHertz;								// ghost����hz
	
	bool use_coordinate_system;								// �Ƿ�ʹ������ϵͳ ���Ϊfalse�� aoi,trap, move�ȹ��ܽ�����ά��
	bool coordinateSystem_hasY;								// ��Χ�������ǹ���Y�ᣬ ע����y����aoi��trap�ȹ������˸߶ȣ� ��y��Ĺ�������һ��������
	uint16 entity_posdir_additional_updates;				// ʵ��λ��ֹͣ�����ı�����������ͻ��˸���tick�ε�λ����Ϣ��Ϊ0�����Ǹ��¡�

	bool aliasEntityID;										// �Ż�EntityID��aoi��Χ��С��255��EntityID, ���䵽clientʱʹ��1�ֽ�αID 
	bool entitydefAliasID;									// �Ż�entity���Ժͷ����㲥ʱռ�õĴ���entity�ͻ������Ի��߿ͻ��˲�����255��ʱ�� ����uid������uid���䵽clientʱʹ��1�ֽڱ���ID

	char internalInterface[MAX_NAME];						// �ڲ������ӿ�����
	char externalInterface[MAX_NAME];						// �ⲿ�����ӿ�����
	char externalAddress[MAX_NAME];							// �ⲿIP��ַ
	int32 externalPorts_min;								// ����socket�˿�ʹ��ָ����Χ
	int32 externalPorts_max;

	std::vector<DBInterfaceInfo> dbInterfaceInfos;			// ���ݿ�ӿ�
	bool notFoundAccountAutoCreate;							// ��¼�Ϸ�ʱ��Ϸ���ݿ��Ҳ�����Ϸ�˺����Զ�����
	bool allowEmptyDigest;									// �Ƿ���defs-MD5
	bool account_registration_enable;						// �Ƿ񿪷�ע��

	float archivePeriod;									// entity�洢���ݿ�����
	float backupPeriod;										// entity��������
	bool backUpUndefinedProperties;							// entity�Ƿ񱸷�δ��������
	uint16 entityRestoreSize;								// entity restoreÿtick���� 

	float loadSmoothingBias;								// baseapp������ƽ�����ֵ�� 
	uint32 login_port;										// ��������¼�˿� Ŀǰbots����
	char login_ip[MAX_BUF];									// ��������¼ip��ַ

	ENTITY_ID criticallyLowSize;							// idʣ����ô���ʱ��dbmgr�����µ�id��Դ

	uint32 downloadBitsPerSecondTotal;						// ���пͻ���ÿ�����ش���������
	uint32 downloadBitsPerSecondPerClient;					// ÿ���ͻ���ÿ������ش���

	Profiles_Config profiles;

	uint32 defaultAddBots_totalCount;						// Ĭ���������̺��Զ������ô���bots ���������
	float defaultAddBots_tickTime;							// Ĭ���������̺��Զ������ô���bots ÿ���������ʱ��(s)
	uint32 defaultAddBots_tickCount;						// Ĭ���������̺��Զ������ô���bots ÿ���������

	std::string bots_account_name_prefix;					// �������˺����Ƶ�ǰ׺
	uint32 bots_account_name_suffix_inc;					// �������˺����Ƶĺ�׺����, 0ʹ������������� ������baseNum��д��������

	uint32 tcp_SOMAXCONN;									// listen�����������ֵ

	int8 encrypt_login;										// ���ܵ�¼��Ϣ

	uint32 telnet_port;
	std::string telnet_passwd;
	std::string telnet_deflayer;

	uint32 perSecsDestroyEntitySize;						// ÿ������base|entity����

	uint64 respool_timeout;
	uint32 respool_buffersize;

	uint8 account_type;										// 1: ��ͨ�˺�, 2: email�˺�(��Ҫ����), 3: �����˺�(�Զ�ʶ��email�� ��ͨ�����) 
	uint32 accountDefaultFlags;								// ���˺�Ĭ�ϱ��(ACCOUNT_FLAGS�ɵ��ӣ� ��дʱ��ʮ���Ƹ�ʽ) 
	uint64 accountDefaultDeadline;							// ���˺�Ĭ�Ϲ���ʱ��(��, �������ϵ�ǰʱ��)
	
	std::string http_cbhost;
	uint16 http_cbport;										// �û�http�ص��ӿڣ�������֤���������õ�

	bool debugDBMgr;										// debugģʽ�¿������д������Ϣ

	bool isOnInitCallPropertysSetMethods;					// ������(bots)ר�ã���Entity��ʼ��ʱ�Ƿ񴥷����Ե�set_*�¼�
} ENGINE_COMPONENT_INFO;

class ServerConfig : public Singleton<ServerConfig>
{
public:
	ServerConfig();
	~ServerConfig();
	
	bool loadConfig(std::string fileName);
	
	INLINE ENGINE_COMPONENT_INFO& getCellApp(void);
	INLINE ENGINE_COMPONENT_INFO& getBaseApp(void);
	INLINE ENGINE_COMPONENT_INFO& getDBMgr(void);
	INLINE ENGINE_COMPONENT_INFO& getLoginApp(void);
	INLINE ENGINE_COMPONENT_INFO& getCellAppMgr(void);
	INLINE ENGINE_COMPONENT_INFO& getBaseAppMgr(void);
	INLINE ENGINE_COMPONENT_INFO& getKBMachine(void);
	INLINE ENGINE_COMPONENT_INFO& getBots(void);
	INLINE ENGINE_COMPONENT_INFO& getLogger(void);
	INLINE ENGINE_COMPONENT_INFO& getInterfaces(void);

	INLINE ENGINE_COMPONENT_INFO& getComponent(COMPONENT_TYPE componentType);
 	
	INLINE ENGINE_COMPONENT_INFO& getConfig();

 	void updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
 				const Network::Address& internalAddr, const Network::Address& externalAddr);
 	
	void updateExternalAddress(char* buf);

	INLINE int16 gameUpdateHertz(void) const;
	INLINE Network::Address interfacesAddr(void) const;

	const ChannelCommon& channelCommon(){ return channelCommon_; }

	uint32 tcp_SOMAXCONN(COMPONENT_TYPE componentType);

	float shutdowntime(){ return shutdown_time_; }
	float shutdownWaitTickTime(){ return shutdown_waitTickTime_; }

	uint32 tickMaxBufferedLogs() const { return tick_max_buffered_logs_; }
	uint32 tickMaxSyncLogs() const { return tick_max_sync_logs_; }

	INLINE bool IsPureDBInterfaceName(const std::string& dbInterfaceName);
	INLINE DBInterfaceInfo* dbInterface(const std::string& name);
	INLINE int dbInterfaceName2dbInterfaceIndex(const std::string& dbInterfaceName);
	INLINE const char* dbInterfaceIndex2dbInterfaceName(size_t dbInterfaceIndex);

private:
	void _updateEmailInfos();

private:
	ENGINE_COMPONENT_INFO _cellAppInfo;
	ENGINE_COMPONENT_INFO _baseAppInfo;
	ENGINE_COMPONENT_INFO _dbmgrInfo;
	ENGINE_COMPONENT_INFO _loginAppInfo;
	ENGINE_COMPONENT_INFO _cellAppMgrInfo;
	ENGINE_COMPONENT_INFO _baseAppMgrInfo;
	ENGINE_COMPONENT_INFO _kbMachineInfo;
	ENGINE_COMPONENT_INFO _botsInfo;
	ENGINE_COMPONENT_INFO _loggerInfo;
	ENGINE_COMPONENT_INFO _interfacesInfo;

public:
	int16 gameUpdateHertz_;
	uint32 tick_max_buffered_logs_;
	uint32 tick_max_sync_logs_;

	ChannelCommon channelCommon_;

	// ÿ���ͻ���ÿ��ռ�õ�������
	uint32 bitsPerSecondToClient_;		

	Network::Address interfacesAddr_;
	uint32 interfaces_orders_timeout_;

	float shutdown_time_;
	float shutdown_waitTickTime_;

	float callback_timeout_;										// callbackĬ�ϳ�ʱʱ��(��)
	float thread_timeout_;											// Ĭ�ϳ�ʱʱ��(��)

	uint32 thread_init_create_, thread_pre_create_, thread_max_create_;
	
	EmailServerInfo	emailServerInfo_;
	EmailSendInfo emailAtivationInfo_;
	EmailSendInfo emailResetPasswordInfo_;
	EmailSendInfo emailBindInfo_;

};

#define g_kbeSrvConfig ServerConfig::getSingleton()
}


#ifdef CODE_INLINE
#include "serverconfig.inl"
#endif
#endif // KBE_SERVER_CONFIG_H
