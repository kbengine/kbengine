#include "event_dispatcher.hpp"

namespace KBEngine { 
namespace Mercury
{

EventDispatcher::EventDispatcher() :
	m_breakProcessing_(false),
	m_maxWait_(0.1),
	m_pFrequentTasks_(new Tasks)
{
}

//-------------------------------------------------------------------------------------
EventDispatcher::~EventDispatcher()
{
	delete m_pFrequentTasks_;
}

//-------------------------------------------------------------------------------------
uint64 EventDispatcher::getSpareTime() const
{
	//return m_pPoller_->spareTime();
	return 0;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::clearSpareTime()
{
	//accSpareTime_ += pPoller_->spareTime();
	//pPoller_->clearSpareTime();
}

//-------------------------------------------------------------------------------------
double EventDispatcher::proportionalSpareTime() const
{
	//double ret = (double)(int64)(totSpareTime_ - oldSpareTime_);
	//return ret/stampsPerSecondD();
	return 0;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::addFrequentTask(Task * pTask)
{
	m_pFrequentTasks_->add(pTask);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::cancelFrequentTask(Task * pTask)
{
	return m_pFrequentTasks_->cancel(pTask);
}

//-------------------------------------------------------------------------------------
double EventDispatcher::calculateWait() const
{
	double maxWait = m_maxWait_;
	/*
	if (!pTimeQueue_->empty())
	{
		maxWait = std::min( maxWait,
			pTimeQueue_->nextExp( timestamp() ) / stampsPerSecondD() );
	}

	ChildDispatchers::const_iterator iter = childDispatchers_.begin();

	while (iter != childDispatchers_.end())
	{
		maxWait = std::min( maxWait, (*iter)->calculateWait() );
		++iter;
	}
	*/
	return maxWait;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processUntilBreak()
{
	this->processContinuously();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processContinuously()
{
	m_breakProcessing_ = false;
	while(!m_breakProcessing_)
	{
		this->processOnce(true);
	}
}

//-------------------------------------------------------------------------------------
int EventDispatcher::processOnce(bool shouldIdle)
{
	m_breakProcessing_ = false;
	this->processFrequentTasks();
	if(!m_breakProcessing_){
		this->processTimers();
	}

	this->processStats();
	
	if(!m_breakProcessing_){
		return this->processNetwork(shouldIdle);
	}

	return 0;
}

}
}