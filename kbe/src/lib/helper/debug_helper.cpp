#include "debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "thread/ThreadGuard.hpp"

namespace KBEngine{
	
KBE_SINGLETON_INIT(DebugHelper);

DebugHelper dbghelper;
log4cxx::LoggerPtr g_logger(log4cxx::Logger::getLogger("default"));

#define DBG_PT_SIZE 1024 * 4
char _g_buf[DBG_PT_SIZE];

#ifdef KBE_USE_ASSERTS
void myassert(const char * exp, const char * func, const char * exp, const char * file, unsigned int line)
{
	sprintf(_g_buf, "assertion failed: %s, file %s, line %d, at: %s\n", exp, file, line, func);
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
_logfile(NULL)
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
	sprintf(helpConfig, "../../res/server/log4cxx_properties/%s.properties", COMPONENT_NAME[componentType]);
	log4cxx::PropertyConfigurator::configure(helpConfig);
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
    _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
    va_end(ap);
	LOG4CXX_INFO(g_logger, _g_buf);
#endif
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
    _vsnprintf(_g_buf, DBG_PT_SIZE, err, ap);
    va_end(ap);
	LOG4CXX_ERROR(g_logger, _g_buf);
#endif
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
    _vsnprintf(_g_buf, DBG_PT_SIZE, info, ap);
    va_end(ap);
	LOG4CXX_INFO(g_logger, _g_buf);
#endif
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
    _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
    va_end(ap);
	LOG4CXX_DEBUG(g_logger, _g_buf);
#endif
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
    _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
    va_end(ap);
	// printf("CRITICAL:%s(%d)\n\t%s\n", _currFile.c_str(), _currLine, _g_buf);
	LOG4CXX_WARN(g_logger, _g_buf);
#endif
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
    _vsnprintf(_g_buf, DBG_PT_SIZE, str, ap);
    va_end(ap);
	char buf[DBG_PT_SIZE];
	sprintf(buf, "%s(%d) -> %s\n\t%s\n", _currFile.c_str(), _currLine, _currFuncName.c_str(), _g_buf);
	// printf(buf);
	LOG4CXX_FATAL(g_logger, buf);
#endif

	setFile("", "", 0);
}
//-------------------------------------------------------------------------------------

}