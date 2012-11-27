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
#include <list> 
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning(disable:4819)
#endif
#include "boost/format.hpp"
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
	class Packet;
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
	if(CTYPE < 0 || ((CTYPE) & (LOG_TYPES)) <= 0)
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

	void print_msg(boost::format& fmt);
	void print_msg(std::string s);

	void debug_msg(boost::format& fmt);
	void debug_msg(std::string s);

	void error_msg(boost::format& fmt);
	void error_msg(std::string s);

	void info_msg(boost::format& fmt);
	void info_msg(std::string s);

	void warning_msg(boost::format& fmt);
	void warning_msg(std::string s);

	void critical_msg(boost::format& fmt);
	void critical_msg(std::string s);

	void script_msg(boost::format& fmt);
	void script_msg(std::string s);

	void onMessage(uint32 logType, const char * str, uint32 length);

	void registerMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr);
	void unregisterMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr);

	void changeLogger(std::string name);

	void clearBufferedLog();
private:
	FILE* _logfile;
	std::string _currFile, _currFuncName;
	uint32 _currLine;
	Mercury::Address messagelogAddr_;
	KBEngine::thread::ThreadMutex logMutex;
	std::list< Mercury::Bundle* > bufferedLogPackets_;
	bool syncStarting_;
	Mercury:: NetworkInterface* pNetworkInterface_;
	Mercury:: EventDispatcher* pDispatcher_;
};

/*---------------------------------------------------------------------------------
	调试信息输出接口
---------------------------------------------------------------------------------*/
#define SCRIPT_MSG(m)					DebugHelper::getSingleton().script_msg((m))									// 输出任何信息
#define PRINT_MSG(m)					DebugHelper::getSingleton().print_msg((m))									// 输出任何信息
#define ERROR_MSG(m)					DebugHelper::getSingleton().error_msg((m))									// 输出一个错误
#define DEBUG_MSG(m)					DebugHelper::getSingleton().debug_msg((m))									// 输出一个debug信息
#define INFO_MSG(m)						DebugHelper::getSingleton().info_msg((m))									// 输出一个info信息
#define WARNING_MSG(m)					DebugHelper::getSingleton().warning_msg((m))								// 输出一个警告信息
#define CRITICAL_MSG(m)					DebugHelper::getSingleton().setFile(__FUNCTION__, \
										__FILE__, __LINE__); \
										DebugHelper::getSingleton().critical_msg((m))

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
