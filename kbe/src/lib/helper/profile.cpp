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

#include "profile.hpp"

namespace KBEngine
{

std::tr1::shared_ptr< ProfileGroup > ProfileGroup::pDefaultGroup_;
TimeStamp ProfileVal::warningPeriod_;

//-------------------------------------------------------------------------------------
uint64 runningTime()
{
	return ProfileGroup::defaultGroup().runningTime();
}

//-------------------------------------------------------------------------------------
ProfileGroup::ProfileGroup()
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
	if (pDefaultGroup_.get() == NULL)
	{
		pDefaultGroup_.reset(new ProfileGroup());
	}

	return *pDefaultGroup_.get();
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
	inProgress_(0)
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
		std::remove( pProfileGroup_->begin(), pProfileGroup_->end(), this );
	}
}

//-------------------------------------------------------------------------------------
} 

