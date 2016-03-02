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


#ifndef KBE_DEBUG_H
#define KBE_DEBUG_H

#include "common/platform.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning(disable:4819)
#endif
#include "common/singleton.h"
#include "thread/threadmutex.h"
#include "network/common.h"
#include "network/address.h"

namespace KBEngine{

namespace Network{
	class Channel;
	class Bundle;
	class EventDispatcher;
	class NetworkInterface;
	class Packet;
}

/** 
	֧��uft-8�����ַ������ 
*/
void vutf8printf(FILE *out, const char *str, va_list* ap);
void utf8printf(FILE *out, const char *str, ...);


#define	KBELOG_UNKNOWN			0x00000000
#define	KBELOG_PRINT			0x00000001
#define	KBELOG_ERROR			0x00000002
#define	KBELOG_WARNING			0x00000004
#define	KBELOG_DEBUG			0x00000008
#define	KBELOG_INFO				0x00000010
#define	KBELOG_CRITICAL			0x00000020
#define KBELOG_SCRIPT_INFO		0x00000040
#define KBELOG_SCRIPT_ERROR		0x00000080
#define KBELOG_SCRIPT_DEBUG		0x00000100
#define KBELOG_SCRIPT_WARNING	0x00000200
#define KBELOG_SCRIPT_NORMAL	0x00000400

#define KBELOG_TYPES KBELOG_UNKNOWN | KBELOG_PRINT | KBELOG_ERROR | KBELOG_WARNING | \
	KBELOG_DEBUG | KBELOG_INFO | KBELOG_CRITICAL | KBELOG_SCRIPT_INFO | KBELOG_SCRIPT_ERROR | KBELOG_SCRIPT_DEBUG | \
	KBELOG_SCRIPT_WARNING | KBELOG_SCRIPT_NORMAL

const char KBELOG_TYPE_NAME[][255] = {
	" UNKNOWN",
	"        ",
	"   ERROR",
	" WARNING",
	"   DEBUG",
	"    INFO",
	"CRITICAL",
	"  S_INFO",
	"  S_ERR",
	"  S_DBG",
	"  S_WARN",
	"  S_NORM",
};

inline const char* KBELOG_TYPE_NAME_EX(uint32 CTYPE)
{									
	if(CTYPE < 0 || ((CTYPE) & (KBELOG_TYPES)) <= 0)
	{
		return " UNKNOWN";
	}
	
	switch(CTYPE)
	{
	case KBELOG_PRINT:
		return "        ";
	case KBELOG_ERROR:
		return "   ERROR";
	case KBELOG_WARNING:
		return " WARNING";
	case KBELOG_DEBUG:
		return "   DEBUG";
	case KBELOG_INFO:
		return "    INFO";
	case KBELOG_CRITICAL:
		return "CRITICAL";
	case KBELOG_SCRIPT_INFO:
		return "  S_INFO";
	case KBELOG_SCRIPT_ERROR:
		return "   S_ERR";
	case KBELOG_SCRIPT_DEBUG:
		return "   S_DBG";
	case KBELOG_SCRIPT_WARNING:
		return "  S_WARN";
	case KBELOG_SCRIPT_NORMAL:
		return "  S_NORM";
	};

	return " UNKNOWN";
}

int KBELOG_TYPE_MAPPING(int type);

class DebugHelper  : public Singleton<DebugHelper>
{
public:
	DebugHelper();

	~DebugHelper();
	
	static bool isInit() { return getSingletonPtr() != 0; }

	static void initialize(COMPONENT_TYPE componentType);
	static void finalise(bool destroy = false);

	void setFile(std::string funcname, std::string file, uint32 line){
		_currFile = file;
		_currLine = line;
		_currFuncName = funcname;
	}
	
	std::string getLogName();

	void lockthread();
	void unlockthread();
    
	void pNetworkInterface(Network::NetworkInterface* networkInterface);
	void pDispatcher(Network::EventDispatcher* dispatcher);
	
	Network::EventDispatcher* pDispatcher() const{ return pDispatcher_; }
	Network::NetworkInterface* pNetworkInterface() const{ return pNetworkInterface_; }

	void print_msg(const std::string& s);
	void debug_msg(const std::string& s);
	void error_msg(const std::string& s);
	void info_msg(const std::string& s);
	void warning_msg(const std::string& s);
	void critical_msg(const std::string& s);
	void script_info_msg(const std::string& s);
	void script_error_msg(const std::string& s);
	void backtrace_msg();

	void onMessage(uint32 logType, const char * str, uint32 length);

	void registerLogger(Network::MessageID msgID, Network::Address* pAddr);
	void unregisterLogger(Network::MessageID msgID, Network::Address* pAddr);

	void changeLogger(const std::string& name);
	void closeLogger();  // close logger for fork + execv

	void clearBufferedLog(bool destroy = false);

	void set_errorcolor();
	void set_normalcolor();
	void set_warningcolor();

	void setScriptMsgType(int msgtype);
	void resetScriptMsgType();

	void shouldWriteToSyslog(bool v = true);

	/** 
		ͬ����־��logger
	*/
	void sync();

	size_t hasBufferedLogPackets() const{ return hasBufferedLogPackets_; }

	Network::Channel* pLoggerChannel();

private:
	FILE* _logfile;
	std::string _currFile, _currFuncName;
	uint32 _currLine;

	Network::Address loggerAddr_;
	KBEngine::thread::ThreadMutex logMutex;

	std::queue< Network::Bundle* > bufferedLogPackets_;
	size_t hasBufferedLogPackets_;

	Network::NetworkInterface* pNetworkInterface_;
	Network::EventDispatcher* pDispatcher_;

	int scriptMsgType_;

	bool noSyncLog_;

	bool canLogFile_;
};

/*---------------------------------------------------------------------------------
	������Ϣ����ӿ�
---------------------------------------------------------------------------------*/
#define SCRIPT_INFO_MSG(m)				DebugHelper::getSingleton().script_info_msg((m))							// ���info��Ϣ
#define SCRIPT_ERROR_MSG(m)				DebugHelper::getSingleton().script_error_msg((m))							// ���������Ϣ

#define PRINT_MSG(m)					DebugHelper::getSingleton().print_msg((m))									// ����κ���Ϣ
#define ERROR_MSG(m)					DebugHelper::getSingleton().error_msg((m))									// ���һ������
#define DEBUG_MSG(m)					DebugHelper::getSingleton().debug_msg((m))									// ���һ��debug��Ϣ
#define INFO_MSG(m)						DebugHelper::getSingleton().info_msg((m))									// ���һ��info��Ϣ
#define WARNING_MSG(m)					DebugHelper::getSingleton().warning_msg((m))								// ���һ��������Ϣ
#define CRITICAL_MSG(m)					DebugHelper::getSingleton().setFile(__FUNCTION__, \
										__FILE__, __LINE__); \
										DebugHelper::getSingleton().critical_msg((m))

/*---------------------------------------------------------------------------------
	���Ժ�
---------------------------------------------------------------------------------*/
#ifdef KBE_USE_ASSERTS
void myassert(const char* exp, const char * func, const char * file, unsigned int line);
#define KBE_ASSERT(exp) if(!(exp))myassert(#exp, __FUNCTION__, __FILE__, __LINE__);
#define KBE_REAL_ASSERT assert(0);
#else
#define KBE_ASSERT(exp) NULL;
#define KBE_REAL_ASSERT
#endif

#ifdef _DEBUG
#define KBE_VERIFY KBE_ASSERT
#else
#define KBE_VERIFY(exp) (exp)
#endif

#define KBE_EXIT(msg) {														\
			CRITICAL_MSG(msg);												\
			KBE_REAL_ASSERT	}												\


}

#endif // KBE_DEBUG_H
