#include "timer.hpp"
#include "helper/debug_helper.hpp"
#include "thread/threadguard.hpp"

namespace KBEngine
{
//-------------------------------------------------------------------------------------
void TimerHandle::cancel()
{
	if (m_pTime_ != NULL)
	{
		TimeBase* pTime = m_pTime_;
		m_pTime_ = NULL;
		pTime->cancel();
	}
}

//-------------------------------------------------------------------------------------
} 
