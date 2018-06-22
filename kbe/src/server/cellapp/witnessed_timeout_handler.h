// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_WITNESSED_TIMEOUT_HANDLER_H
#define KBE_WITNESSED_TIMEOUT_HANDLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"
// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

class WitnessedTimeoutHandler : public TimerHandler
{	
public:	
	WitnessedTimeoutHandler();

	~WitnessedTimeoutHandler();

	void addWitnessed(Entity* pEntity);
	void delWitnessed(Entity* pEntity);

private:
	virtual void handleTimeout(TimerHandle handle, void * pUser);

	virtual void onRelease( TimerHandle handle, void * /*pUser*/ ){};

	void cancel();

	std::map<ENTITY_ID, uint16>		witnessedEntityIDs_;
	TimerHandle* pTimerHandle_;
};	


}

#endif // KBE_WITNESSED_TIMEOUT_HANDLER_H
