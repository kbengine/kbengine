// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_CLIENT_CONFIG_H
#define KBE_CLIENT_CONFIG_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif

#include "common/singleton.h"
#include "thread/threadmutex.h"
#include "thread/threadguard.h"
#include "xml/xml.h"	
	
namespace KBEngine{

class Config : public Singleton<Config>
{
public:
	Config();
	~Config();
	
	bool loadConfig(std::string fileName);
	
	inline int16 gameUpdateHertz(void) const { return gameUpdateHertz_;}

	uint32 tcp_SOMAXCONN();

	const char* entryScriptFile() const{ return &entryScriptFile_[0]; }

	const char* accountName() const{ return &accountName_[0]; }

	const char* ip() const{ return &ip_[0]; }
	uint32 port() const{ return port_; }

	void writeAccountName(const char* name);

	bool useLastAccountName() const{ return useLastAccountName_; }

	int8 encryptLogin() const { return encrypt_login_; }
	
	bool isOnInitCallPropertysSetMethods() const { return isOnInitCallPropertysSetMethods_; }

public:
	int16 gameUpdateHertz_;

	uint32 tcp_SOMAXCONN_;									// listen监听队列最大值
	
	uint32 port_;											// 组件的运行后监听的端口
	char ip_[MAX_BUF];										// 组件的运行期ip地址

	char entryScriptFile_[MAX_NAME];						// 组件的入口脚本文件

	float channelInternalTimeout_;
	float channelExternalTimeout_;

	char accountName_[MAX_NAME];
	
	int8 encrypt_login_;

	std::string fileName_;

	bool useLastAccountName_;

	uint32 telnet_port;
	std::string telnet_passwd;
	std::string telnet_deflayer;

	bool isOnInitCallPropertysSetMethods_;
};

#define g_kbeConfig KBEngine::Config::getSingleton()
}
#endif
