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

namespace KBEngine{
	
KBE_SINGLETON_INIT(DebugHelper);

DebugHelper dbghelper;
log4cxx::LoggerPtr g_logger(log4cxx::Logger::getLogger("default"));

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
watcherChannels_(),
logMutex()
{
}

//-------------------------------------------------------------------------------------
DebugHelper::~DebugHelper()
{
}	

//-------------------------------------------------------------------------------------
void DebugHelper::initHelper(COMPONENT_TYPE componentType)
{
	g_logger = log4cxx::Logger::getLogger(COMPONENT_NAME[componentType]);
	char helpConfig[256];

	if(componentType == CLIENT_TYPE)
	{
		kbe_snprintf(helpConfig, 256, "log4j.properties");
	}
	else
	{
		kbe_snprintf(helpConfig, 256, "server/log4cxx_properties/%s.properties", COMPONENT_NAME[componentType]);
	}

	log4cxx::PropertyConfigurator::configure(Resmgr::matchRes(helpConfig).c_str());
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
    fprintf(file,"[%-4d-%02d-%02d %02d:%02d:%02d] ",aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
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
    printf("[%02d:%02d:%02d] ",aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
}

//-------------------------------------------------------------------------------------
void DebugHelper::onMessage(LOG_TYPE logType, const char * str)
{
	int strlength = strlen(str);
	if(strlength <= 0)
		return;

	WATCH_CHANNELS::iterator iter = watcherChannels_.begin();
	for(; iter != watcherChannels_.end(); iter++)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle) << (*iter).first;

		Mercury::MessageLength len = sizeof(logType) + strlength;
		(*pBundle) << len;
		(*pBundle) << logType;
		(*pBundle) << str;
		(*iter).second->send(pBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void DebugHelper::registerWatch(Mercury::MessageID msgID, Mercury::Channel* pChannel)
{
	watcherChannels_.push_back(std::make_pair<Mercury::MessageID, Mercury::Channel*>(msgID, pChannel));
}

//-------------------------------------------------------------------------------------
void DebugHelper::unregisterWatch(Mercury::MessageID msgID, Mercury::Channel* pChannel)
{
	WATCH_CHANNELS::iterator iter = watcherChannels_.begin();
	for(; iter != watcherChannels_.end(); iter++)
	{
		if((msgID == (*iter).first || msgID == 0) && (*iter).second == pChannel)
		{
			watcherChannels_.erase(iter);
			return;
		}
	}
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
	_vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	LOG4CXX_INFO(g_logger, _g_buf);
#endif

	onMessage(LOG_PRINT, _g_buf);
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
	_vsnprintf(_g_buf, DBG_PT_SIZE, err, ap);
#else
    vsnprintf(_g_buf, DBG_PT_SIZE, err, ap);
#endif
    va_end(ap);
	LOG4CXX_ERROR(g_logger, _g_buf);
#endif

	onMessage(LOG_ERROR, _g_buf);
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
	_vsnprintf(_g_buf, DBG_PT_SIZE, info, ap);
#else
    vsnprintf(_g_buf, DBG_PT_SIZE, info, ap);
#endif
    va_end(ap);
	LOG4CXX_INFO(g_logger, _g_buf);
#endif

	onMessage(LOG_INFO, _g_buf);
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
	_vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	LOG4CXX_DEBUG(g_logger, _g_buf);
#endif

	onMessage(LOG_DEBUG, _g_buf);
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
	_vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	// printf("CRITICAL:%s(%d)\n\t%s\n", _currFile.c_str(), _currLine, _g_buf);
	LOG4CXX_WARN(g_logger, _g_buf);
#endif

	onMessage(LOG_WARNING, _g_buf);
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
	_vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#else
    vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
#endif
    va_end(ap);
	char buf[DBG_PT_SIZE];
	kbe_snprintf(buf, DBG_PT_SIZE, "%s(%d) -> %s\n\t%s\n", _currFile.c_str(), _currLine, _currFuncName.c_str(), _g_buf);
	// printf(buf);
	LOG4CXX_FATAL(g_logger, buf);
#endif

	setFile("", "", 0);

	onMessage(LOG_CRITICAL, _g_buf);
}
//-------------------------------------------------------------------------------------

}
