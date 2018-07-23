// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PYPROFILE_HANDLER_H
#define KBE_PYPROFILE_HANDLER_H

#include "helper/profile_handler.h"

namespace KBEngine { 

class PyProfileHandler : public ProfileHandler
{
public:
	PyProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr);
	virtual ~PyProfileHandler();
	
	void timeout();
	void sendStream(MemoryStream* s);
};

class PyTickProfileHandler : public Task,
                             public ProfileHandler
{
public:
	PyTickProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen,
		std::string name, const Network::Address& addr);
	virtual ~PyTickProfileHandler();

	virtual void timeout();
	virtual bool process();
	virtual void sendStream(MemoryStream* s);
};


}

#endif // KBE_PYPROFILE_HANDLER_H
