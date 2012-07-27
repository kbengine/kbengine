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


#include "forward_messagebuffer.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

namespace KBEngine { 
KBE_SINGLETON_INIT(Forward_MessageBuffer);

//-------------------------------------------------------------------------------------
Forward_MessageBuffer::Forward_MessageBuffer(Mercury::NetworkInterface & networkInterface, COMPONENT_TYPE forwardComponentType) :
	Task(),
	networkInterface_(networkInterface),
	forwardComponentType_(forwardComponentType),
	start_(false)
{
	// dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
Forward_MessageBuffer::~Forward_MessageBuffer()
{
	//dispatcher().cancelFrequentTask(this);
}

//-------------------------------------------------------------------------------------
Mercury::EventDispatcher & Forward_MessageBuffer::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
void Forward_MessageBuffer::push(Mercury::Bundle* pBundle)
{
	if(!start_)
	{
		dispatcher().addFrequentTask(this);
		start_ = true;
	}
	
	pBundles_.push_back(pBundle);
}

//-------------------------------------------------------------------------------------
bool Forward_MessageBuffer::process()
{
	if(pBundles_.size() <= 0)
	{
		start_ = false;
		return false;
	}
	
	Components::COMPONENTS cts = Components::getSingleton().getComponents(forwardComponentType_);
	size_t idx = 0;

	if(cts.size() > 0)
	{
		// 必须所有的组件频道都被设置， 如果不是则等待。
		Components::COMPONENTS::iterator ctiter = cts.begin();
		for(; ctiter != cts.end(); ctiter++)
		{
			if((*ctiter).pChannel == NULL)
				return true;
		}

		std::vector<Mercury::Bundle*>::iterator iter = pBundles_.begin();
		for(; iter != pBundles_.end(); iter++)
		{
			(*iter)->send(networkInterface_, cts[idx].pChannel);

			idx++;
			if(idx >= cts.size())
				idx = 0;

			SAFE_RELEASE((*iter));
		}
		
		pBundles_.clear();
		start_ = false;
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------
}