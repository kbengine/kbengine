#include "timer.hpp"
#include "log/debug_helper.hpp"
#include "thread/threadguard.hpp"

namespace KBEngine
{
//-------------------------------------------------------------------------------------
void TimerHandle::cancel()
{
	if (m_pTimer_ != NULL)
	{
		TimerBase* pTimer = m_pTimer_;
		m_pTimer_ = NULL;
		pTimer->cancel();
	}
}

//-------------------------------------------------------------------------------------
} 
