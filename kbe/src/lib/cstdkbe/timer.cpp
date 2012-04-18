#include "timer.hpp"
#include "helper/debug_helper.hpp"
#include "thread/threadguard.hpp"

namespace KBEngine
{
//-------------------------------------------------------------------------------------
void TimerHandle::cancel()
{
	if (pTime_ != NULL)
	{
		TimeBase* pTime = pTime_;
		pTime_ = NULL;
		pTime->cancel();
	}
}

//-------------------------------------------------------------------------------------
} 
