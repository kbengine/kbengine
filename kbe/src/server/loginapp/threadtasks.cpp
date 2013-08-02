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

#include "threadtasks.hpp"
#include "loginapp.hpp"
#include "jwsmtp.h"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/deadline.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
SendActivateEMailTask::~SendActivateEMailTask()
{
}

//-------------------------------------------------------------------------------------
bool SendActivateEMailTask::process()
{
	Deadline maildeadline(g_kbeSrvConfig.emailAtivationInfo_.deadline);
	std::wstring wdeadline = maildeadline.wprint();

	jwsmtp::mailer m(emailaddr_.c_str(), g_kbeSrvConfig.emailServerInfo_.username.c_str(), g_kbeSrvConfig.emailAtivationInfo_.subject.c_str(),
		g_kbeSrvConfig.emailAtivationInfo_.subject.c_str(), g_kbeSrvConfig.emailServerInfo_.smtp_server.c_str(),
		g_kbeSrvConfig.emailServerInfo_.smtp_port, false);

	m.authtype(g_kbeSrvConfig.emailServerInfo_.smtp_auth == 1 ? jwsmtp::mailer::LOGIN : jwsmtp::mailer::PLAIN);  

	std::string mailmessage = g_kbeSrvConfig.emailAtivationInfo_.message;

	KBEngine::strutil::kbe_replace(mailmessage, "${backlink}", (boost::format("http://%1%:%2%/accountactivate?%3%") % 
		cbaddr_ %
		cbport_ %
		code_).str());

	m.setmessageHTML(mailmessage);

	m.username(g_kbeSrvConfig.emailServerInfo_.username.c_str());
	m.password(g_kbeSrvConfig.emailServerInfo_.password.c_str());
	m.send(); // send the mail

	INFO_MSG(boost::format("SendActivateEMailTask::process: sendmail: %1%\n") % m.response());
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState SendActivateEMailTask::presentMainThread()
{
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
}
