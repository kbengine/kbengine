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
#include "network/tcp_packet.hpp"

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
	printf("%s", _g_buf);
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
bufferedLogPackets_(),
syncStarting_(false),
pNetworkInterface_(NULL),
pDispatcher_(NULL)
{
}

//-------------------------------------------------------------------------------------
DebugHelper::~DebugHelper()
{
	clearBufferedLog();
}	

//-------------------------------------------------------------------------------------
void DebugHelper::changeLogger(std::string name)
{
#ifndef NO_USE_LOG4CXX
	g_logger = log4cxx::Logger::getLogger(name.c_str());
#endif
}

//-------------------------------------------------------------------------------------
void DebugHelper::lockthread()
{
	logMutex.lockMutex();
}

//-------------------------------------------------------------------------------------
void DebugHelper::unlockthread()
{
	logMutex.unlockMutex();
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
#endif
}

//-------------------------------------------------------------------------------------
void DebugHelper::clearBufferedLog()
{
	std::list< Mercury::Bundle* >::iterator iter = bufferedLogPackets_.begin();
	for(; iter != bufferedLogPackets_.end(); iter++)
	{
		delete (*iter);
	}

	bufferedLogPackets_.clear();
}

//-------------------------------------------------------------------------------------
void DebugHelper::sync()
{
	if(bufferedLogPackets_.size() == 0)
		return;

	if(messagelogAddr_.isNone())
	{
		if(bufferedLogPackets_.size() > 128)
		{
			ERROR_MSG("DebugHelper::sync: can't found messagelog. packet size=%u.\n", bufferedLogPackets_.size());
			clearBufferedLog();
		}
		return;
	}

	int8 v = Mercury::g_trace_packet;
	Mercury::g_trace_packet = 0;

	Mercury::Channel* pChannel = pNetworkInterface_->findChannel(messagelogAddr_);
	if(pChannel == NULL)
	{
		if(bufferedLogPackets_.size() > 1024)
		{
			messagelogAddr_.ip = 0;
			messagelogAddr_.port = 0;

			WARNING_MSG("DebugHelper::sync: is no use the messagelog, packet size=%u.\n", 
				bufferedLogPackets_.size());
			clearBufferedLog();
		}

		Mercury::g_trace_packet = v;
		return;
	}

	if(bufferedLogPackets_.size() > 0)
	{
		if(bufferedLogPackets_.size() > 32)
		{
			WARNING_MSG("DebugHelper::sync: packet size=%u.\n", bufferedLogPackets_.size());
		}

		int i = 0;

		size_t totalLen = 0;

		std::list< Mercury::Bundle* >::iterator iter = bufferedLogPackets_.begin();
		for(; iter != bufferedLogPackets_.end();)
		{
			if(i++ >= 32 || totalLen > (PACKET_MAX_SIZE_TCP * 10))
				break;
			
			totalLen += (*iter)->currMsgLength();
			pChannel->send((*iter));
			
			Mercury::Bundle::ObjPool().reclaimObject((*iter));
			bufferedLogPackets_.erase(iter++); 
		}
	}

	Mercury::g_trace_packet = v;
}

//-------------------------------------------------------------------------------------
bool DebugHelper::process()
{
	if(bufferedLogPackets_.size() == 0 || pNetworkInterface_ == NULL)
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

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	int8 v = Mercury::g_trace_packet;
	Mercury::g_trace_packet = 0;
	pBundle->newMessage(MessagelogInterface::writeLog);

	(*pBundle) << logType;
	(*pBundle) << g_componentType;
	(*pBundle) << g_componentID;
	(*pBundle) << g_componentOrder;

	int64 t = time(NULL);
	(*pBundle) << t;
	(*pBundle) << g_kbetime;
	(*pBundle) << str;
	
	bufferedLogPackets_.push_back(pBundle);

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

	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

#ifdef NO_USE_LOG4CXX
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

	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

#ifdef NO_USE_LOG4CXX
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

	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

#ifdef NO_USE_LOG4CXX
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

	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

#ifdef NO_USE_LOG4CXX
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

	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

#ifdef NO_USE_LOG4CXX
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

	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

#ifdef NO_USE_LOG4CXX
#else
    va_list ap;
    va_start(ap, str);
#if KBE_PLATFORM == PLATFORM_WIN32
	uint32 size = _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    uint32 size = vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);

	LOG4CXX_WARN(g_logger, _g_buf);
#endif

	onMessage(LOG_WARNING, _g_buf, size);
}

void DebugHelper::critical_msg(const char * str, ...)
{
    if(str == NULL)
        return;

	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

#ifdef NO_USE_LOG4CXX
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
	LOG4CXX_FATAL(g_logger, buf);
#endif

	setFile("", "", 0);

	onMessage(LOG_CRITICAL, _g_buf, size);
}
//-------------------------------------------------------------------------------------

}


