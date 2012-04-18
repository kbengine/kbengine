#include "debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "thread/ThreadGuard.hpp"

namespace KBEngine{
template<> DebugHelper* Singleton<DebugHelper>::singleton_ = 0;
DebugHelper dbghelper;

#ifdef KBE_USE_ASSERTS
void myassert(const char * exp, const char * file, unsigned int line)
{
    printf("assertion failed: %s, file %s, line %d\n",exp, file, line);
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
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    if(!str)
        return;

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
}

//-------------------------------------------------------------------------------------
void DebugHelper::error_msg(const char * err, ...)
{
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    if(!err)
        return;

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
}

//-------------------------------------------------------------------------------------
void DebugHelper::info_msg(const char * info, ...)
{
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    if(!info)
        return;

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
}

//-------------------------------------------------------------------------------------
void DebugHelper::debug_msg(const char * str, ...)
{
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    if(!str)
        return;

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
}

//-------------------------------------------------------------------------------------
void DebugHelper::warning_msg(const char * str, ...)
{
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    if(!str)
        return;

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
}

void DebugHelper::critical_msg(const char * str, ...)
{
	KBEngine::thread::ThreadGuard tg(&this->logMutex); 

    if(!str)
        return;

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

	setFile("", 0);
}
//-------------------------------------------------------------------------------------

}