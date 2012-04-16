/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORK_INTERFACES__
#define __NETWORK_INTERFACES__

namespace KBEngine { 
namespace Mercury
{
class Bundle;

/** 此类接口用于接收普通的Mercury消息
*/
class InputNotificationHandler
{
public:
	virtual ~InputNotificationHandler() {};
	virtual int handleInputNotification(int fd) = 0;
};

class BundlePrimer
{
public:
	virtual ~BundlePrimer() {}
	virtual void primeBundle(Bundle & bundle) = 0;
	virtual int numUnreliableMessages() const = 0;
};

}
}
#endif // __NETWORK_INTERFACES__