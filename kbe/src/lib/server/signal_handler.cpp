#include "signal_handler.hpp"
#include "helper/debug_helper.hpp"

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


//-------------------------------------------------------------------------------------
SignalHandlers::SignalHandlers()
{
}

//-------------------------------------------------------------------------------------
SignalHandlers::~SignalHandlers()
{
}
	
//-------------------------------------------------------------------------------------	
SignalHandler* SignalHandlers::addSignal(int sigNum, 
	SignalHandler* pSignalHandler)
{
	SignalHandlerMap::iterator iter = singnalHandlerMap_.find(sigNum);
	KBE_ASSERT(iter == singnalHandlerMap_.end());
	singnalHandlerMap_[sigNum] = pSignalHandler;
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
void SignalHandlers::process()
{
	std::vector<int>::iterator iter = signalledVec_.begin();
	for(; iter != signalledVec_.end(); iter++)
	{
		int sigNum = (*iter);
		SignalHandlerMap::iterator iter = singnalHandlerMap_.find(sigNum);
		if(iter == singnalHandlerMap_.end())
		{
			DEBUG_MSG("SignalHandlers::process: sigNum %d unhandled.\n", sigNum);
			continue;
		}
		
		iter->second->onHandle(sigNum);
		DEBUG_MSG("SignalHandlers::process: sigNum %d handled.\n", sigNum);
	}
}

//-------------------------------------------------------------------------------------		
}
