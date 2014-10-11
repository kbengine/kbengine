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


#ifndef KBE_DEBUG_HPP
#define KBE_DEBUG_HPP

#include <assert.h>
#include <time.h>	
#include <stdarg.h> 
#include <queue> 
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#pragma warning(disable:4819)
#endif
#include "cstdkbe/singleton.hpp"
#include "thread/threadmutex.hpp"
#include "network/common.hpp"
#include "network/address.hpp"

namespace KBEngine{

namespace Mercury{
	class Channel;
	class Bundle;
	class EventDispatcher;
	class NetworkInterface;
	class Packet;
}

/** 
	支持uft-8编码字符串输出 
*/
void vutf8printf(FILE *out, const char *str, va_list* ap);
void utf8printf(FILE *out, const char *str, ...);


#define	KBELOG_UNKNOWN		0x00000000
#define	KBELOG_PRINT		0x00000001
#define	KBELOG_ERROR		0x00000002
#define	KBELOG_WARNING		0x00000004
#define	KBELOG_DEBUG		0x00000008
#define	KBELOG_INFO			0x00000010
#define	KBELOG_CRITICAL		0x00000020
#define KBELOG_SCRIPT		0x00000040

#define KBELOG_TYPES KBELOG_UNKNOWN | KBELOG_PRINT | KBELOG_ERROR | KBELOG_WARNING | KBELOG_DEBUG | KBELOG_INFO | KBELOG_CRITICAL | KBELOG_SCRIPT

const char KBELOG_TYPE_NAME[][255] = {
	" UNKNOWN",
	"        ",
	"   ERROR",
	" WARNING",
	"   DEBUG",
	"    INFO",
	"CRITICAL",
	"  SCRIPT",
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
	case KBELOG_SCRIPT:
		return "  SCRIPT";
	};

	return " UNKNOWN";
}

class DebugHelper  : public Singleton<DebugHelper>
{
public:
	DebugHelper();

	~DebugHelper();
	
	static bool isInit() { return getSingletonPtr() != 0; }

	static void initHelper(COMPONENT_TYPE componentType);

	void setFile(std::string funcname, std::string file, uint32 line){
		_currFile = file;
		_currLine = line;
		_currFuncName = funcname;
	}
	
	std::string getLogName();

	void lockthread();
	void unlockthread();
    
	void pNetworkInterface(Mercury:: NetworkInterface* networkInterface);
	void pDispatcher(Mercury:: EventDispatcher* dispatcher);
	
	Mercury:: EventDispatcher* pDispatcher()const{ return pDispatcher_; }
	Mercury:: NetworkInterface* pNetworkInterface()const{ return pNetworkInterface_; }

	void print_msg(const std::string& s);
	void debug_msg(const std::string& s);
	void error_msg(const std::string& s);
	void info_msg(const std::string& s);
	void warning_msg(const std::string& s);
	void critical_msg(const std::string& s);
	void script_msg(const std::string& s);
	void backtrace_msg();

	void onMessage(uint32 logType, const char * str, uint32 length);

	void registerMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr);
	void unregisterMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr);

	void changeLogger(std::string name);

	void clearBufferedLog(bool destroy = false);

	void setScriptMsgType(int msgtype);

	void shouldWriteToSyslog(bool v = true);

	/** 
		同步日志到messagelog
	*/
	void sync();
private:
	FILE* _logfile;
	std::string _currFile, _currFuncName;
	uint32 _currLine;

	Mercury::Address messagelogAddr_;
	KBEngine::thread::ThreadMutex logMutex;

	std::queue< Mercury::Bundle* > bufferedLogPackets_;
	size_t hasBufferedLogPackets_;

	Mercury:: NetworkInterface* pNetworkInterface_;
	Mercury:: EventDispatcher* pDispatcher_;

	int scriptMsgType_;

	bool noSyncLog_;

	bool canLogFile_;
};

/*---------------------------------------------------------------------------------
	调试信息输出接口
---------------------------------------------------------------------------------*/
#define SCRIPT_MSG(m)	DebugHelper::getSingleton().setFile(__FUNCTION__, __FILE__, __LINE__); DebugHelper::getSingleton().script_msg((m))		// 输出任何信息
#define PRINT_MSG(m)	DebugHelper::getSingleton().setFile(__FUNCTION__, __FILE__, __LINE__); DebugHelper::getSingleton().print_msg((m))		// 输出任何信息
#define ERROR_MSG(m)	DebugHelper::getSingleton().setFile(__FUNCTION__, __FILE__, __LINE__); DebugHelper::getSingleton().error_msg((m))		// 输出一个错误
#define DEBUG_MSG(m)	DebugHelper::getSingleton().setFile(__FUNCTION__, __FILE__, __LINE__); DebugHelper::getSingleton().debug_msg((m))		// 输出一个debug信息
#define INFO_MSG(m)		DebugHelper::getSingleton().setFile(__FUNCTION__, __FILE__, __LINE__); DebugHelper::getSingleton().info_msg((m))		// 输出一个info信息
#define WARNING_MSG(m)	DebugHelper::getSingleton().setFile(__FUNCTION__, __FILE__, __LINE__); DebugHelper::getSingleton().warning_msg((m))		// 输出一个警告信息
#define CRITICAL_MSG(m)	DebugHelper::getSingleton().setFile(__FUNCTION__, __FILE__, __LINE__); DebugHelper::getSingleton().critical_msg((m))

/*---------------------------------------------------------------------------------
	调试宏
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

#endif // KBE_DEBUG_HPP
