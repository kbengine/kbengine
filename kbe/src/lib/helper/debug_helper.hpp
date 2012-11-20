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


#ifndef __KBE_DEBUG_HPP__
#define __KBE_DEBUG_HPP__

#include <assert.h>
#include <time.h>	
#include <stdarg.h> 
#include "cstdkbe/tasks.hpp"
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
}

/** 
	支持uft-8编码字符串输出 
*/
void vutf8printf(FILE *out, const char *str, va_list* ap);
void utf8printf(FILE *out, const char *str, ...);


#define	LOG_UNKNOWN			0x00000000
#define	LOG_PRINT			0x00000001
#define	LOG_ERROR			0x00000002
#define	LOG_WARNING			0x00000004
#define	LOG_DEBUG			0x00000008
#define	LOG_INFO			0x00000010
#define	LOG_CRITICAL		0x00000020
#define LOG_SCRIPT			0x00000040

#define LOG_TYPES LOG_UNKNOWN | LOG_PRINT | LOG_ERROR | LOG_WARNING | LOG_DEBUG | LOG_INFO | LOG_CRITICAL | LOG_SCRIPT

const char LOG_TYPE_NAME[][255] = {
	" UNKNOWN",
	"        ",
	"   ERROR",
	" WARNING",
	"   DEBUG",
	"    INFO",
	"CRITICAL",
	"  SCRIPT",
};

inline const char* LOG_TYPE_NAME_EX(uint32 CTYPE)
{									
	if(CTYPE < 0 || (CTYPE & LOG_TYPES) <= 0)
	{
		return " UNKNOWN";
	}
	
	switch(CTYPE)
	{
	case LOG_PRINT:
		return "        ";
	case LOG_ERROR:
		return "   ERROR";
	case LOG_WARNING:
		return " WARNING";
	case LOG_DEBUG:
		return "   DEBUG";
	case LOG_INFO:
		return "    INFO";
	case LOG_CRITICAL:
		return "CRITICAL";
	case LOG_SCRIPT:
		return "  SCRIPT";
	};

	return " UNKNOWN";
}

class DebugHelper : public Task, 
					public Singleton<DebugHelper>
{
public:
	DebugHelper();

	~DebugHelper();
	
	static void initHelper(COMPONENT_TYPE componentType);

	void setFile(std::string funcname, std::string file, uint32 line){
		_currFile = file;
		_currLine = line;
		_currFuncName = funcname;
	}
	
	void lockthread();
	void unlockthread();

	/** 
		同步日志到messagelog
	*/
	void sync();
	bool process();
    
	void pNetworkInterface(Mercury:: NetworkInterface* networkInterface);
	void pDispatcher(Mercury:: EventDispatcher* dispatcher);

	void print_msg(const char * str, ...);
    void debug_msg(const char * str, ...);
    void error_msg(const char * err, ...);
    void info_msg(const char * info, ...);
	void warning_msg(const char * str, ...);
	void critical_msg(const char * str, ...);
	void script_msg(const char * str, ...);
	void onMessage(uint32 logType, const char * str, uint32 length);

	void registerMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr);
	void unregisterMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr);

	void changeLogger(std::string name);
private:
	FILE* _logfile;
	std::string _currFile, _currFuncName;
	uint32 _currLine;
	Mercury::Address messagelogAddr_;
	KBEngine::thread::ThreadMutex logMutex;
	Mercury::Bundle* pBundle_;
	bool syncStarting_;
	Mercury:: NetworkInterface* pNetworkInterface_;
	Mercury:: EventDispatcher* pDispatcher_;
};

/*---------------------------------------------------------------------------------
	调试信息输出接口
---------------------------------------------------------------------------------*/
#define SCRIPT_MSG					DebugHelper::getSingleton().script_msg									// 输出任何信息
#define PRINT_MSG					DebugHelper::getSingleton().print_msg									// 输出任何信息
#define ERROR_MSG					DebugHelper::getSingleton().error_msg									// 输出一个错误
#define DEBUG_MSG					DebugHelper::getSingleton().debug_msg									// 输出一个debug信息
#define INFO_MSG					DebugHelper::getSingleton().info_msg									// 输出一个info信息
#define WARNING_MSG					DebugHelper::getSingleton().warning_msg									// 输出一个警告信息
#define CRITICAL_MSG				DebugHelper::getSingleton().setFile(__FUNCTION__, \
									__FILE__, __LINE__); \
									DebugHelper::getSingleton().critical_msg

/*---------------------------------------------------------------------------------
	调试宏
---------------------------------------------------------------------------------*/
#ifdef KBE_USE_ASSERTS
void myassert(const char* exp, const char * func, const char * file, unsigned int line);
#define KBE_ASSERT(exp) if(!(exp))myassert(#exp, __FUNCTION__, __FILE__, __LINE__);
#else
#define KBE_ASSERT(exp) NULL;
#endif

#ifdef _DEBUG
#define KBE_VERIFY KBE_ASSERT
#else
#define KBE_VERIFY(exp) (exp)
#endif

}

#endif // __KBE_DEBUG_HPP__
