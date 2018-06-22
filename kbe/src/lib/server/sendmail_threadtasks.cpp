// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "jwsmtp.h"
#include "sendmail_threadtasks.h"
#include "server/serverconfig.h"
#include "common/deadline.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
bool SendEMailTask::process()
{
	jwsmtp::mailer m(emailaddr_.c_str(), g_kbeSrvConfig.emailServerInfo_.username.c_str(), subject(),
		g_kbeSrvConfig.emailAtivationInfo_.subject.c_str(), g_kbeSrvConfig.emailServerInfo_.smtp_server.c_str(),
		g_kbeSrvConfig.emailServerInfo_.smtp_port, false);

	m.authtype(g_kbeSrvConfig.emailServerInfo_.smtp_auth == 1 ? jwsmtp::mailer::LOGIN : jwsmtp::mailer::PLAIN);  

	std::string mailmessage = message();

	KBEngine::strutil::kbe_replace(mailmessage, "${backlink}", fmt::format("http://{}:{}/{}_{}", 
		cbaddr_,
		cbport_,
		getopkey(),
		code_));

	KBEngine::strutil::kbe_replace(mailmessage, "${username}", emailaddr_);
	KBEngine::strutil::kbe_replace(mailmessage, "${code}", code_);
	mailmessage = KBEngine::strutil::kbe_trim(mailmessage);

	m.setmessageHTML(mailmessage);

	m.username(g_kbeSrvConfig.emailServerInfo_.username.c_str());
	m.password(g_kbeSrvConfig.emailServerInfo_.password.c_str());
	m.send(); // send the mail

	INFO_MSG(fmt::format("SendEMailTask::process: sendmail[{}]: {}\n", getopkey(), m.response()));
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState SendEMailTask::presentMainThread()
{
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
const char* SendActivateEMailTask::subject()
{
	return g_kbeSrvConfig.emailAtivationInfo_.subject.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendActivateEMailTask::message()
{
	return g_kbeSrvConfig.emailAtivationInfo_.message.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendResetPasswordEMailTask::subject()
{
	return g_kbeSrvConfig.emailResetPasswordInfo_.subject.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendResetPasswordEMailTask::message()
{
	return g_kbeSrvConfig.emailResetPasswordInfo_.message.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendBindEMailTask::subject()
{
	return g_kbeSrvConfig.emailBindInfo_.subject.c_str();
}

//-------------------------------------------------------------------------------------
const char* SendBindEMailTask::message()
{
	return g_kbeSrvConfig.emailBindInfo_.message.c_str();
}

//-------------------------------------------------------------------------------------
}
