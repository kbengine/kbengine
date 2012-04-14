/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBE_DEBUG_HPP__
#define __KBE_DEBUG_HPP__

#include <assert.h>
#include <time.h>	
#include <stdarg.h> 
#include "cstdkbe/singleton.hpp"
#include "thread/ThreadMutex.hpp"
namespace KBEngine{

/** 支持uft-8编码字符串输出 */
void vutf8printf(FILE *out, const char *str, va_list* ap);
void utf8printf(FILE *out, const char *str, ...);

class DebugHelper : public Singleton<DebugHelper>
{
private:
	FILE* _m_logfile;
	std::string _m_currFile;
	uint32 _m_currLine;
public:
	KBEngine::thread::ThreadMutex logMutex;
public:
	DebugHelper()
	{
		_m_logfile = NULL;
	}

	~DebugHelper()
	{
	}	
	
	void setFile(std::string file, uint32 line){
		_m_currFile = file;
		_m_currLine = line;
	}

	void outTime();
	static void outTimestamp(FILE* file);
    
	void print_msg(const char * str, ...);
    void debug_msg(const char * str, ...);
    void error_msg(const char * err, ...);
    void info_msg(const char * info, ...);
	void warning_msg(const char * str, ...);
	void critical_msg(const char * str, ...);
};

/*---------------------------------------------------------------------------------
	调试信息输出接口
---------------------------------------------------------------------------------*/
#define PRINT_MSG			DebugHelper::getSingleton().print_msg									// 输出任何信息
#define ERROR_MSG			DebugHelper::getSingleton().error_msg									// 输出一个错误
#define DEBUG_MSG			DebugHelper::getSingleton().debug_msg									// 输出一个debug信息
#define INFO_MSG			DebugHelper::getSingleton().info_msg									// 输出一个info信息
#define WARNING_MSG			DebugHelper::getSingleton().warning_msg									// 输出一个警告信息
#define CRITICAL_MSG		DebugHelper::getSingleton().setFile(__FILE__, __LINE__); \
							DebugHelper::getSingleton().critical_msg
/*---------------------------------------------------------------------------------
	调试宏
---------------------------------------------------------------------------------*/
#ifdef KBE_USE_ASSERTS
void myassert(const char* exp, const char * file, unsigned int line);
#define KBE_ASSERT(exp) if(!(exp))myassert(#exp, __FILE__, __LINE__);
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
