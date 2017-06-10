/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#include "profile.h"
#include "helper/watcher.h"

#ifndef CODE_INLINE
#include "profile.inl"
#endif


namespace KBEngine
{

ProfileGroup* g_pDefaultGroup = NULL;
TimeStamp ProfileVal::warningPeriod_;

//-------------------------------------------------------------------------------------
uint64 runningTime()
{
	return ProfileGroup::defaultGroup().runningTime();
}

//-------------------------------------------------------------------------------------
ProfileGroup::ProfileGroup(std::string name):
name_(name)
{
	stampsPerSecond();

	ProfileVal * pRunningTime = new ProfileVal("RunningTime", this);
	pRunningTime->start();
}

//-------------------------------------------------------------------------------------
ProfileGroup::~ProfileGroup()
{
	delete this->pRunningTime();
}

//-------------------------------------------------------------------------------------
void ProfileGroup::finalise(void)
{
	SAFE_RELEASE(g_pDefaultGroup);
}

//-------------------------------------------------------------------------------------
TimeStamp ProfileGroup::runningTime() const
{
	return timestamp() - this->pRunningTime()->lastTime_;
}

//-------------------------------------------------------------------------------------
void ProfileGroup::add( ProfileVal * pVal )
{
	profiles_.push_back( pVal );
}

//-------------------------------------------------------------------------------------
ProfileGroup & ProfileGroup::defaultGroup()
{
	if(g_pDefaultGroup == NULL)
		g_pDefaultGroup = new ProfileGroup();
	return *g_pDefaultGroup;
}

//-------------------------------------------------------------------------------------
bool ProfileGroup::initializeWatcher()
{
	return true;
}

//-------------------------------------------------------------------------------------
ProfileVal::ProfileVal(std::string name, ProfileGroup * pGroup):
	name_(name),
	pProfileGroup_(pGroup),
	lastTime_(0),
	sumTime_(0),
	lastIntTime_(0),
	sumIntTime_(0),
	lastQuantity_(0),
	sumQuantity_(0),
	count_(0),
	inProgress_(0),
	initWatcher_(false)
{
	if (pProfileGroup_ == NULL)
	{
		pProfileGroup_ = &ProfileGroup::defaultGroup();
	}

	if (!name_.empty())
	{
		pProfileGroup_->add( this );
	}
}

//-------------------------------------------------------------------------------------
ProfileVal::~ProfileVal()
{
	if (pProfileGroup_)
	{
		// pProfileGroup_.erase(std::remove( pProfileGroup_->begin(), pProfileGroup_->end(), this ), pProfileGroup_->end());
	}
}

//-------------------------------------------------------------------------------------
bool ProfileVal::initializeWatcher()
{
	if(initWatcher_)
		return false;

	initWatcher_ = true;

	char buf[MAX_BUF];
	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/lastTime", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, &lastTime_, &TimeStamp::stamp);

	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/sumTime", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, &sumTime_, &TimeStamp::stamp);

	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/lastIntTime", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, &lastIntTime_, &TimeStamp::stamp);

	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/sumIntTime", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, &sumIntTime_, &TimeStamp::stamp);

	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/lastQuantity", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, lastQuantity_);

	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/sumQuantity", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, sumQuantity_);

	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/count", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, count_);

	kbe_snprintf(buf, MAX_BUF, "cprofiles/%s/%s/inProgress", pProfileGroup_->name(), name_.c_str());
	WATCH_OBJECT(buf, inProgress_);

	return true;
}

//-------------------------------------------------------------------------------------
} 

