// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SENDMAIL_THREADTASKS_H
#define KBE_SENDMAIL_THREADTASKS_H

#include "common/common.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"
#include "network/address.h"

namespace KBEngine{ 

class SendEMailTask : public thread::TPTask
{
public:
	SendEMailTask(const std::string& emailaddr, const std::string& code, const std::string& cbaddr, uint32 cbport):
	emailaddr_(emailaddr),
	code_(code),
	cbaddr_(cbaddr),
	cbport_(cbport)
	{
	}

	virtual ~SendEMailTask(){}
	virtual bool process();
	virtual thread::TPTask::TPTaskState presentMainThread();
	virtual const char* getopkey() = 0;
	virtual const char* subject() = 0;
	virtual const char* message() = 0;

protected:
	std::string emailaddr_, code_, cbaddr_;
	uint32 cbport_;
};

/*
	账号激活邮件发送线程任务
*/

class SendActivateEMailTask : public SendEMailTask
{
public:
	SendActivateEMailTask(const std::string& emailaddr, const std::string& code, const std::string& cbaddr, uint32 cbport):
	SendEMailTask(emailaddr, code, cbaddr, cbport)
	{
	}

	virtual ~SendActivateEMailTask(){}

	virtual const char* getopkey(){ return "accountactivate"; }
	virtual const char* subject();
	virtual const char* message();

protected:

};

/*
	忘记密码邮件发送线程任务
*/

class SendResetPasswordEMailTask : public SendEMailTask
{
public:
	SendResetPasswordEMailTask(const std::string& emailaddr, const std::string& code, const std::string& cbaddr, uint32 cbport):
	SendEMailTask(emailaddr, code, cbaddr, cbport)
	{
	}

	virtual ~SendResetPasswordEMailTask(){}

	virtual const char* getopkey(){ return "resetpassword"; }
	virtual const char* subject();
	virtual const char* message();

protected:

};

/*
	账号绑定邮箱邮件发送线程任务
*/

class SendBindEMailTask : public SendEMailTask
{
public:
	SendBindEMailTask(const std::string& emailaddr, const std::string& code, const std::string& cbaddr, uint32 cbport):
	SendEMailTask(emailaddr, code, cbaddr, cbport)
	{
	}

	virtual ~SendBindEMailTask(){}

	virtual const char* getopkey(){ return "bindmail"; }
	virtual const char* subject();
	virtual const char* message();

protected:

};


}

#endif // KBE_SENDMAIL_THREADTASKS_H
