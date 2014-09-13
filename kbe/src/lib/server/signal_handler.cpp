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
	DEBUG_MSG(boost::format("SignalHandlers: receive sigNum %1%.\n") % SIGNAL_NAMES[signum]);
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
	app->mainDispatcher().addFrequentTask(this);
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
	if(signalledVec_.size() > 0)
	{
		std::vector<int>::iterator iter = signalledVec_.begin();
		for(; iter != signalledVec_.end(); iter++)
		{
			int sigNum = (*iter);
			SignalHandlerMap::iterator iter1 = singnalHandlerMap_.find(sigNum);
			if(iter1 == singnalHandlerMap_.end())
			{
				DEBUG_MSG(boost::format("SignalHandlers::process: sigNum %1% unhandled, singnalHandlerMap(%2%).\n") % 
					SIGNAL_NAMES[sigNum] % singnalHandlerMap_.size());
				continue;
			}
			
			DEBUG_MSG(boost::format("SignalHandlers::process: sigNum %1% handle.\n") % SIGNAL_NAMES[sigNum]);
			iter1->second->onSignalled(sigNum);
		}

		signalledVec_.clear();
	}

	return true;
}

//-------------------------------------------------------------------------------------		
}
