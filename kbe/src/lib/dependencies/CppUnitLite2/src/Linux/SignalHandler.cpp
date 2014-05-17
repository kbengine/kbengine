#include "SignalHandler.h"
#include <signal.h>
#include <setjmp.h>


namespace {

sigjmp_buf g_sigMark;
struct sigaction m_old_SIGFPE_action;
struct sigaction m_old_SIGTRAP_action;
struct sigaction m_old_SIGSEGV_action;
struct sigaction m_old_SIGBUS_action;
struct sigaction m_old_SIGABRT_action;
struct sigaction m_old_SIGALRM_action;

void signal_handler (int sig)
{
    siglongjmp(g_sigMark, sig );
}

}


SignalHandler::SignalHandler(bool bCatchSystemExceptions) 
    : m_bCatchSystemExceptions (bCatchSystemExceptions)
{
    if (!m_bCatchSystemExceptions)
        return;
        
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = &signal_handler;
    sigemptyset( &action.sa_mask );
       
    sigaction( SIGSEGV, &action, &m_old_SIGSEGV_action );
    sigaction( SIGFPE , &action, &m_old_SIGFPE_action  );
    sigaction( SIGTRAP, &action, &m_old_SIGTRAP_action );
    sigaction( SIGBUS , &action, &m_old_SIGBUS_action  );
    sigaction( SIGABRT, &action, &m_old_SIGABRT_action );        
    
    if (sigsetjmp( g_sigMark, 1 ) == 0)              
        return;
        
    throw ("Unhandled system exception");    
}

SignalHandler::~SignalHandler()
{
    if (!m_bCatchSystemExceptions)
        return;
        
    sigaction( SIGABRT, &m_old_SIGABRT_action, 0 );
    sigaction( SIGBUS , &m_old_SIGBUS_action , 0 );
    sigaction( SIGTRAP, &m_old_SIGTRAP_action, 0 );
    sigaction( SIGFPE , &m_old_SIGFPE_action , 0 );
    sigaction( SIGSEGV, &m_old_SIGSEGV_action, 0 );

}

