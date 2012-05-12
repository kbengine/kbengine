#include "signal_handler.hpp"
#include "helper/debug_helper.hpp"
#include "server/serverapp.hpp"

namespace KBEngine{
KBE_SINGLETON_INIT(SignalHandlers);

const int SIGMIN = 1;
const int SIGMAX = SIGSYS;

const char * SIGNAL_NAMES[] = 
{
	NULL,
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGABRT",
	"SIGBUS",
	"SIGFPE",
	"SIGKILL",
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM",
	"SIGSTKFLT",
	"SIGCHLD",
	"SIGCONT",
	"SIGSTOP",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
	"SIGURG",
	"SIGXCPU",
	"SIGXFSZ",
	"SIGVTALRM",
	"SIGPROF",
	"SIGWINCH",
	"SIGIO",
	"SIGPWR",
	"SIGSYS"
};

SignalHandlers g_signalHandlers;

void signalHandler(int signum)
{
	DEBUG_MSG("SignalHandlers: receive sigNum %s.\n", SIGNAL_NAMES[signum]);
	g_signalHandlers.onSignalled(signum);
};

//-------------------------------------------------------------------------------------
SignalHandlers::SignalHandlers():
singnalHandlerMap_(),
signalledVec_(),
papp_(NULL)
{
}

//-------------------------------------------------------------------------------------
SignalHandlers::~SignalHandlers()
{
}

//-------------------------------------------------------------------------------------
void SignalHandlers::attachApp(ServerApp* app)
{ 
	papp_ = app; 
	app->getMainDispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------	
SignalHandler* SignalHandlers::addSignal(int sigNum, 
	SignalHandler* pSignalHandler, int flags)
{
	SignalHandlerMap::iterator iter = singnalHandlerMap_.find(sigNum);
	KBE_ASSERT(iter == singnalHandlerMap_.end());
	singnalHandlerMap_[sigNum] = pSignalHandler;

#if KBE_PLATFORM != PLATFORM_WIN32
	struct sigaction action;
	action.sa_handler = &signalHandler;
	sigfillset( &(action.sa_mask) );

	if (flags & SA_SIGINFO)
	{
		ERROR_MSG( "ServerApp::installSingnal: "
				"SA_SIGINFO is not supported, ignoring\n" );
		flags &= ~SA_SIGINFO;
	}

	action.sa_flags = flags;

	::sigaction( sigNum, &action, NULL );
#endif

	return pSignalHandler;
}
	
//-------------------------------------------------------------------------------------	
SignalHandler* SignalHandlers::delSignal(int sigNum)
{
	SignalHandlerMap::iterator iter = singnalHandlerMap_.find(sigNum);
	KBE_ASSERT(iter != singnalHandlerMap_.end());
	SignalHandler* pSignalHandler = iter->second;
	singnalHandlerMap_.erase(iter);
	return pSignalHandler;
}
	
//-------------------------------------------------------------------------------------	
void SignalHandlers::clear()
{
	singnalHandlerMap_.clear();
}

//-------------------------------------------------------------------------------------	
void SignalHandlers::onSignalled(int sigNum)
{
	signalledVec_.push_back(sigNum);
}

//-------------------------------------------------------------------------------------	
bool SignalHandlers::process()
{
	std::vector<int>::iterator iter = signalledVec_.begin();
	for(; iter != signalledVec_.end(); iter++)
	{
		int sigNum = (*iter);
		SignalHandlerMap::iterator iter1 = singnalHandlerMap_.find(sigNum);
		if(iter1 == singnalHandlerMap_.end())
		{
			DEBUG_MSG("SignalHandlers::process: sigNum %s unhandled, singnalHandlerMap(%u).\n", 
				SIGNAL_NAMES[sigNum], singnalHandlerMap_.size());
			continue;
		}
		
		DEBUG_MSG("SignalHandlers::process: sigNum %s handle.\n", SIGNAL_NAMES[sigNum]);
		iter1->second->onSignalled(sigNum);
	}

	signalledVec_.clear();
	return true;
}

//-------------------------------------------------------------------------------------		
}
