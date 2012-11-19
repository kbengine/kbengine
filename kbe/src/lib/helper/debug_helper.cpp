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


#include "debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "thread/threadguard.hpp"
#include "network/channel.hpp"
#include "resmgr/resmgr.hpp"
#include "network/bundle.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

#ifndef NO_USE_LOG4CXX
#include "log4cxx/logger.h"
#include "log4cxx/net/socketappender.h"
#include "log4cxx/helpers/inetaddress.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/logstring.h"
#include "log4cxx/basicconfigurator.h"
#include "helper/script_loglevel.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma comment (lib, "Mswsock.lib")
#pragma comment( lib, "odbc32.lib" )
#ifdef _DEBUG
#pragma comment (lib, "apr_d.lib")
#pragma comment (lib, "aprutil_d.lib")
#pragma comment (lib, "expat_d.lib")
#pragma comment (lib, "log4cxx_d.lib")
#else
#pragma comment (lib, "apr.lib")
#pragma comment (lib, "aprutil.lib")
#pragma comment (lib, "expat.lib")
#pragma comment (lib, "log4cxx.lib")
#endif
#endif
#endif

#include "../../server/tools/message_log/messagelog_interface.hpp"

namespace KBEngine{
	
KBE_SINGLETON_INIT(DebugHelper);

DebugHelper dbghelper;


#ifndef NO_USE_LOG4CXX
log4cxx::LoggerPtr g_logger(log4cxx::Logger::getLogger("default"));
#endif

#define DBG_PT_SIZE 1024 * 4
char _g_buf[DBG_PT_SIZE];

#ifdef KBE_USE_ASSERTS
void myassert(const char * exp, const char * func, const char * file, unsigned int line)
{
	kbe_snprintf(_g_buf, DBG_PT_SIZE, "assertion failed: %s, file %s, line %d, at: %s\n", exp, file, line, func);
    dbghelper.print_msg(_g_buf);
	printf(_g_buf);
    abort();
}
#endif

//-------------------------------------------------------------------------------------
void utf8printf(FILE *out, const char *str, ...)
{
    va_list ap;
    va_start(ap, str);
    vutf8printf(stdout, str, &ap);
    va_end(ap);
}

//-------------------------------------------------------------------------------------
void vutf8printf(FILE *out, const char *str, va_list* ap)
{
    vfprintf(out, str, *ap);
}

//-------------------------------------------------------------------------------------
DebugHelper::DebugHelper():
_logfile(NULL),
_currFile(),
_currFuncName(),
_currLine(0),
messagelogAddr_(),
logMutex(),
pBundle_(NULL),
syncStarting_(false),
pNetworkInterface_(NULL),
pDispatcher_(NULL)
{
}

//-------------------------------------------------------------------------------------
DebugHelper::~DebugHelper()
{
	SAFE_RELEASE(pBundle_);
}	

//-------------------------------------------------------------------------------------
void DebugHelper::changeLogger(std::string name)
{
	g_logger = log4cxx::Logger::getLogger(name.c_str());
}

//-------------------------------------------------------------------------------------
void DebugHelper::initHelper(COMPONENT_TYPE componentType)
{
#ifndef NO_USE_LOG4CXX
	g_logger = log4cxx::Logger::getLogger(COMPONENT_NAME_EX(componentType));
	char helpConfig[256];

	if(componentType == CLIENT_TYPE)
	{
		kbe_snprintf(helpConfig, 256, "log4j.properties");
	}
	else
	{
		kbe_snprintf(helpConfig, 256, "server/log4cxx_properties/%s.properties", COMPONENT_NAME_EX(componentType));
	}

	log4cxx::PropertyConfigurator::configure(Resmgr::getSingleton().matchRes(helpConfig).c_str());

	//log4cxx::helpers::InetAddressPtr aaa = new log4cxx::helpers::InetAddress(log4cxx::LogString(L"localhost"), log4cxx::LogString(L"192.168.4.205")); 
	
	//log4cxx::net::SocketAppenderPtr bbb = new log4cxx::net::SocketAppender(aaa, 6593);
	
	//bbb->setLayout(new log4cxx::PatternLayout(L"%5p [%t] [%d] - %m"));
	//log4cxx::BasicConfigurator::configure(bbb); 
	//bbb->setLocationInfo(true);
	//bbb->setReconnectionDelay(100);

	// log4cxx::helpers::Pool p;
	//bbb->activateOptions(p);
	//log4cxx::Logger::getRootLogger()->addAppender(bbb);
	
#endif
}

//-------------------------------------------------------------------------------------
void DebugHelper::outTimestamp(FILE* file)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    fprintf(file, "[%-4d-%02d-%02d %02d:%02d:%02d] ",aTm->tm_year+1900, aTm->tm_mon+1, 
		aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
}

//-------------------------------------------------------------------------------------
void DebugHelper::outTime()
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    printf("[%02d:%02d:%02d] ", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
}

//-------------------------------------------------------------------------------------
void DebugHelper::sync()
{
	if(pBundle_ == NULL)
		return;

	if(messagelogAddr_.isNone())
	{
		if(pBundle_->packets().size() > 128)
		{
			ERROR_MSG("DebugHelper::sync: can't found messagelog. packet size=%u.\n", pBundle_->packets().size());
			Mercury::Bundle::ObjPool().reclaimObject(pBundle_);
			pBundle_ = NULL;
		}
		return;
	}

	int8 v = Mercury::g_trace_packet;
	Mercury::g_trace_packet = 0;

	static uint16 trycount = 0;
	Mercury::Channel* pChannel = pNetworkInterface_->findChannel(messagelogAddr_);
	if(pChannel == NULL)
	{
		if(trycount++ > 512)
		{
			messagelogAddr_.ip = 0;
			messagelogAddr_.port = 0;
		}
		Mercury::g_trace_packet = v;
		return;
	}

	pChannel->send(pBundle_);
	
	Mercury::Bundle::ObjPool().reclaimObject(pBundle_);
	pBundle_ = NULL;
	Mercury::g_trace_packet = v;
}

//-------------------------------------------------------------------------------------
bool DebugHelper::process()
{
	if(pBundle_ == NULL || pNetworkInterface_ == NULL)
	{
		syncStarting_ = false;
		return false;
	}

	sync();
	return true;
}

//-------------------------------------------------------------------------------------
void DebugHelper::pDispatcher(Mercury:: EventDispatcher* dispatcher)
{ 
	pDispatcher_ = dispatcher; 
	if(syncStarting_)
	{
		pDispatcher_->addFrequentTask(this);
	}
}

//-------------------------------------------------------------------------------------
void DebugHelper::pNetworkInterface(Mercury:: NetworkInterface* networkInterface)
{ 
	pNetworkInterface_ = networkInterface; 
}

//-------------------------------------------------------------------------------------
void DebugHelper::onMessage(uint32 logType, const char * str, uint32 length)
{
	if(g_componentType == MACHINE_TYPE || 
		g_componentType == CONSOLE_TYPE || g_componentType == MESSAGELOG_TYPE)
		return;

	if(length <= 0)
		return;

	if(pBundle_ == NULL)
		pBundle_ = Mercury::Bundle::ObjPool().createObject();

	int8 v = Mercury::g_trace_packet;
	Mercury::g_trace_packet = 0;
	pBundle_->newMessage(MessagelogInterface::writeLog);

	(*pBundle_) << logType;
	(*pBundle_) << g_componentType;
	(*pBundle_) << g_componentID;
	(*pBundle_) << g_componentOrder;

	int64 t = time(NULL);
	(*pBundle_) << t;
	(*pBundle_) << g_kbetime;
	(*pBundle_) << str;
	
	Mercury::g_trace_packet = v;
	if(!syncStarting_)
	{
		if(pDispatcher_)
			pDispatcher_->addFrequentTask(this);
		syncStarting_ = true;
	}
}

//-------------------------------------------------------------------------------------
void DebugHelper::registerMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr)
{
	messagelogAddr_ = *pAddr;
}

//-------------------------------------------------------------------------------------
void DebugHelper::unregisterMessagelog(Mercury::MessageID msgID, Mercury::Address* pAddr)
{
	messagelogAddr_.ip = 0;
	messagelogAddr_.port = 0;
}

//-------------------------------------------------------------------------------------
void DebugHelper::print_msg(const char * str, ...)
{
    if(str == NULL)
        return;

#ifdef NO_USE_LOG4CXX
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    va_list ap;
    va_start(ap, str);
    vutf8printf(stdout, str, &ap);
    va_end(ap);

    if(_logfile)
    {
        outTimestamp(_logfile);
        va_start(ap, str);
        vfprintf(_logfile, str, ap);
        fprintf(_logfile, "\n");
        va_end(ap);
        fflush(_logfile);
    }

    fflush(stdout);
#else
    va_list ap;
    va_start(ap, str);

#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	LOG4CXX_INFO(g_logger, _g_buf);
#endif

	onMessage(LOG_PRINT, _g_buf, size);
}

//-------------------------------------------------------------------------------------
void DebugHelper::error_msg(const char * err, ...)
{
    if(err == NULL)
        return;

#ifdef NO_USE_LOG4CXX
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    outTime();
	fprintf(stderr, "ERROR:");

    va_list ap;
    va_start(ap, err);
    vutf8printf(stderr, err, &ap);
    va_end(ap);

    if(_logfile)
    {
        outTimestamp(_logfile);
        fprintf(_logfile, "ERROR:");

        va_start(ap, err);
        vfprintf(_logfile, err, ap);
        va_end(ap);

        fprintf(_logfile, "\n");
        fflush(_logfile);
    }

    fflush(stderr);
#else
    va_list ap;
    va_start(ap, err);
#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, err, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, err, ap);
#endif
    va_end(ap);
	LOG4CXX_ERROR(g_logger, _g_buf);
#endif

	onMessage(LOG_ERROR, _g_buf, size);
}

//-------------------------------------------------------------------------------------
void DebugHelper::info_msg(const char * info, ...)
{
    if(info == NULL)
        return;

#ifdef NO_USE_LOG4CXX
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    outTime();
	fprintf(stdout, "INFO:");

    va_list ap;
    va_start(ap, info);
    vutf8printf(stdout, info, &ap);
    va_end(ap);

    if(_logfile)
    {
        outTimestamp(_logfile);
        fprintf(_logfile, "INFO:");

        va_start(ap, info);
        vfprintf(_logfile, info, ap);
        va_end(ap);

        fprintf(_logfile, "\n");
        fflush(_logfile);
    }

    fflush(stdout);
#else
    va_list ap;
    va_start(ap, info);
#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, info, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, info, ap);
#endif
    va_end(ap);
	LOG4CXX_INFO(g_logger, _g_buf);
#endif

	onMessage(LOG_INFO, _g_buf, size);
}

//-------------------------------------------------------------------------------------
void DebugHelper::script_msg(const char * info, ...)
{
    if(info == NULL)
        return;

#ifdef NO_USE_LOG4CXX
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    outTime();
	fprintf(stdout, "SCRIPT:");

    va_list ap;
    va_start(ap, info);
    vutf8printf(stdout, info, &ap);
    va_end(ap);

    if(_logfile)
    {
        outTimestamp(_logfile);
        fprintf(_logfile, "SCRIPT:");

        va_start(ap, info);
        vfprintf(_logfile, info, ap);
        va_end(ap);

        fprintf(_logfile, "\n");
        fflush(_logfile);
    }

    fflush(stdout);
#else
    va_list ap;
    va_start(ap, info);
#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, info, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, info, ap);
#endif
    va_end(ap);
	LOG4CXX_LOG(g_logger, log4cxx::ScriptLevel::getScript(), _g_buf);
#endif

	onMessage(LOG_SCRIPT, _g_buf, size);
}

//-------------------------------------------------------------------------------------
void DebugHelper::debug_msg(const char * str, ...)
{
    if(str == NULL)
        return;

#ifdef NO_USE_LOG4CXX
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    outTime();
	fprintf(stdout, "DEBUG:");

    va_list ap;
    va_start(ap, str);
    vutf8printf(stdout, str, &ap);
    va_end(ap);

    if(_logfile)
    {
        outTimestamp(_logfile);
        fprintf(_logfile, "DEBUG:");

        va_start(ap, str);
        vfprintf(_logfile, str, ap);
        va_end(ap);

        fprintf(_logfile, "\n");
        fflush(_logfile);
    }

    fflush(stdout);
#else 
    va_list ap;
    va_start(ap, str);
#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	LOG4CXX_DEBUG(g_logger, _g_buf);
#endif

	onMessage(LOG_DEBUG, _g_buf, size);
}

//-------------------------------------------------------------------------------------
void DebugHelper::warning_msg(const char * str, ...)
{
    if(str == NULL)
        return;

#ifdef NO_USE_LOG4CXX
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    outTime();
	fprintf(stdout, "WARNING:");

    va_list ap;
    va_start(ap, str);
    vutf8printf(stdout, str, &ap);
    va_end(ap);

    if(_logfile)
    {
        outTimestamp(_logfile);
        fprintf(_logfile, "WARNING:");

        va_start(ap, str);
        vfprintf(_logfile, str, ap);
        va_end(ap);

        fprintf(_logfile, "\n");
        fflush(_logfile);
    }

    fflush(stdout);
#else
    va_list ap;
    va_start(ap, str);
#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	// printf("CRITICAL:%s(%d)\n\t%s\n", _currFile.c_str(), _currLine, _g_buf);
	LOG4CXX_WARN(g_logger, _g_buf);
#endif

	onMessage(LOG_WARNING, _g_buf, size);
}

void DebugHelper::critical_msg(const char * str, ...)
{
    if(str == NULL)
        return;

#ifdef NO_USE_LOG4CXX
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    outTime();
	fprintf(stdout, "CRITICAL:%s(%d)\n\t", _currFile.c_str(), _currLine);

    va_list ap;
    va_start(ap, str);
    vutf8printf(stdout, str, &ap);
    va_end(ap);

    if(_logfile)
    {
        outTimestamp(_logfile);
		fprintf(_logfile, "CRITICAL:%s(%d)\n\t", _currFile.c_str(), _currLine);

        va_start(ap, str);
        vfprintf(_logfile, str, ap);
        va_end(ap);

        fprintf(_logfile, "\n");
        fflush(_logfile);
    }

    fflush(stdout);
#else
    va_list ap;
    va_start(ap, str);
#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	char buf[DBG_PT_SIZE];
	kbe_snprintf(buf, DBG_PT_SIZE, "%s(%d) -> %s\n\t%s\n", _currFile.c_str(), _currLine, _currFuncName.c_str(), _g_buf);
	// printf(buf);
	LOG4CXX_FATAL(g_logger, buf);
#endif

	setFile("", "", 0);

	onMessage(LOG_CRITICAL, _g_buf, size);
}
//-------------------------------------------------------------------------------------

}


