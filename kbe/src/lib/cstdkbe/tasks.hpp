/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef __TASKS__
#define __TASKS__
#include "cstdkbe/cstdkbe.hpp"

namespace KBEngine
{

/**
 *	抽象一个任务
 */
class Task
{
public:
	virtual ~Task() {}

	virtual void process() = 0;
};



/**
 *	抽象一个任务容器
 */
class Tasks
{
public:
	Tasks();
	~Tasks();

	void add(Task * pTask);
	bool cancel(Task * pTask);
	void process();
private:
	
	typedef std::vector<KBEngine::Task *> Container;
	Container container_;
};

}

#endif // __TASKS__
