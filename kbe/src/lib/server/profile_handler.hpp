/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PROFILE_HANDLER__
#define __PROFILE_HANDLER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/tasks.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine { 
namespace Mercury
{
class NetworkInterface;
class Address;
}

class ProfileHandler : public TimerHandler
{
public:
	ProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	virtual ~ProfileHandler();
	
	virtual void timeout() = 0;

	static KBEUnordered_map<std::string, KBEShared_ptr< ProfileHandler > > profiles;
protected:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	Mercury::NetworkInterface& networkInterface_;

	TimerHandle reportLimitTimerHandle_;
	
	std::string name_;
	
	Mercury::Address addr_;
};

class PyProfileHandler : public ProfileHandler
{
public:
	PyProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	~PyProfileHandler();
	
	void timeout();
};

class CProfileHandler : public Task, 
						public ProfileHandler
{
public:
	CProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	~CProfileHandler();
	
	void timeout();
	
	bool process();
private:
	struct ProfileVal
	{
		// 名称
		std::string		name;

		// startd后的时间.
		TimeStamp		lastTime;
		TimeStamp		diff_lastTime;

		// count_次的总时间
		TimeStamp		sumTime;
		TimeStamp		diff_sumTime;

		// 记录最后一次内部时间片
		TimeStamp		lastIntTime;
		TimeStamp		diff_lastIntTime;

		// count_次内部总时间
		TimeStamp		sumIntTime;
		TimeStamp		diff_sumIntTime;

		uint32			count;
		TimeStamp		diff_count;
	};

	// 此ProfileVal只在计时器开始时记录default.profiles的初始值
	// 在结束时取出差值得到结果
	typedef KBEUnordered_map<std::string,  ProfileVal> PROFILEVALS;
	PROFILEVALS profileVals_;
};

class EventProfileHandler : public ProfileHandler
{
public:
	EventProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	~EventProfileHandler();
	
	void timeout();
};

}

#endif
