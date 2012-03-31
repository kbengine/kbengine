/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __EVENT_POLLER__
#define __EVENT_POLLER__

#include "network/interfaces.hpp"
#include <map>
namespace KBEngine { 
namespace Mercury
{
	
class InputNotificationHandler;
typedef std::map<int, InputNotificationHandler *> FDHandlers;

class EventPoller:public InputNotificationHandler
{
public:
	EventPoller();
	virtual ~EventPoller();

	bool registerForRead(int fd, InputNotificationHandler * handler);
	bool registerForWrite(int fd, InputNotificationHandler * handler);

	bool deregisterForRead(int fd);
	bool deregisterForWrite(int fd);


	virtual int processPendingEvents(double maxWait) = 0;
	virtual int getFileDescriptor() const;
	virtual int handleInputNotification(int fd);

	void clearSpareTime()		{spareTime_ = 0;}
	uint64 spareTime() const	{return spareTime_;}

	static EventPoller * create();

protected:
	virtual bool doRegisterForRead( int fd ) = 0;
	virtual bool doRegisterForWrite( int fd ) = 0;

	virtual bool doDeregisterForRead( int fd ) = 0;
	virtual bool doDeregisterForWrite( int fd ) = 0;

	bool triggerRead( int fd )	{return this->trigger( fd, fdReadHandlers_ );}
	bool triggerWrite( int fd )	{return this->trigger( fd, fdWriteHandlers_ );}
	bool triggerError( int fd );

	bool trigger( int fd, FDHandlers & handlers );
	
	bool isRegistered( int fd, bool isForRead ) const;
	
	int maxFD() const;

private:
	static int maxFD(const FDHandlers & handlerMap);
	FDHandlers fdReadHandlers_;
	FDHandlers fdWriteHandlers_;

protected:
	uint64 spareTime_;
};

}
}
#endif // __EVENT_POLLER__