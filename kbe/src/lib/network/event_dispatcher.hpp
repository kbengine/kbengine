/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __EVENT_DISPATCHER__
#define __EVENT_DISPATCHER__

#include <map>
#include "cstdkbe/tasks.hpp"

namespace KBEngine { 
namespace Mercury
{
	
class EventDispatcher
{
public:
	EventDispatcher();
	virtual ~EventDispatcher();
	
	void processContinuously();
	int  processOnce(bool shouldIdle = false);
	void processUntilBreak();

	void breakProcessing(bool breakState = true);
	bool processingBroken()const;
	
	void attach(EventDispatcher & childDispatcher);
	void detach(EventDispatcher & childDispatcher);
	
	void addFrequentTask(Task * pTask);
	bool cancelFrequentTask(Task * pTask);
	
	inline double maxWait() const;
	inline void maxWait(double seconds);

	uint64 getSpareTime() const;
	void clearSpareTime();
	double proportionalSpareTime() const;
private:
	void processFrequentTasks();
	void processTimers();
	void processStats();
	int processNetwork(bool shouldIdle);
	
	double calculateWait() const;
	
	void attachTo(EventDispatcher & parentDispatcher);
	void detachFrom(EventDispatcher & parentDispatcher);
protected:
	bool m_breakProcessing_;
	double m_maxWait_;
	
	Tasks* m_pFrequentTasks_;
};

}
}

#include "event_dispatcher.ipp"
#endif // __EVENT_DISPATCHER__