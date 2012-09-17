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
#include "cstdkbe/singleton.hpp"
#include "thread/threadmutex.hpp"
#include "network/common.hpp"
#ifndef NO_USE_LOG4CXX
#include "log4cxx/logger.h"
#include "log4cxx/propertyconfigurator.h"
#endif

namespace KBEngine{

namespace Mercury{
	class Channel;
}

/** 
	支持uft-8编码字符串输出 
*/
void vutf8printf(FILE *out, const char *str, va_list* ap);
void utf8printf(FILE *out, const char *str, ...);

class DebugHelper : public Singleton<DebugHelper>
{
public:
	enum LOG_TYPE
	{
		LOG_PRINT = 0,
		LOG_ERROR = 1,
		LOG_WARNING = 2,
		LOG_DEBUG = 3,
		LOG_INFO = 4,
		LOG_CRITICAL = 5
	};

	typedef std::vector<std::pair<Mercury::MessageID, Mercury::Channel*> > WATCH_CHANNELS;
public:
	DebugHelper();

	~DebugHelper();
	
	static void initHelper(COMPONENT_TYPE componentType);

	void setFile(std::string funcname, std::string file, uint32 line){
		_currFile = file;
		_currLine = line;
		_currFuncName = funcname;
	}

	void outTime();
	static void outTimestamp(FILE* file);
    
	void print_msg(const char * str, ...);
    void debug_msg(const char * str, ...);
    void error_msg(const char * err, ...);
    void info_msg(const char * info, ...);
	void warning_msg(const char * str, ...);
	void critical_msg(const char * str, ...);

	void onMessage(LOG_TYPE logType, const char * str);

	void registerWatch(Mercury::MessageID msgID, Mercury::Channel* pChannel);
	void unregisterWatch(Mercury::MessageID msgID, Mercury::Channel* pChannel);
private:
	FILE* _logfile;
	std::string _currFile, _currFuncName;
	uint32 _currLine;
	WATCH_CHANNELS watcherChannels_;
	KBEngine::thread::ThreadMutex logMutex;
};

/*---------------------------------------------------------------------------------
	调试信息输出接口
---------------------------------------------------------------------------------*/
#define PRINT_MSG					DebugHelper::getSingleton().print_msg									// 输出任何信息
#define ERROR_MSG					DebugHelper::getSingleton().setFile(__FUNCTION__, \
									__FILE__, __LINE__); \
									DebugHelper::getSingleton().error_msg									// 输出一个错误
#define DEBUG_MSG					DebugHelper::getSingleton().setFile(__FUNCTION__, \
									__FILE__, __LINE__); \
									DebugHelper::getSingleton().debug_msg									// 输出一个debug信息
#define INFO_MSG					DebugHelper::getSingleton().info_msg									// 输出一个info信息
#define WARNING_MSG					DebugHelper::getSingleton().setFile(__FUNCTION__, \
									__FILE__, __LINE__); \
									DebugHelper::getSingleton().warning_msg									// 输出一个警告信息
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
