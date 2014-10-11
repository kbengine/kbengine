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
KBE_SINGLETON_INIT(ForwardComponent_MessageBuffer);
KBE_SINGLETON_INIT(ForwardAnywhere_MessageBuffer);
//-------------------------------------------------------------------------------------
ForwardComponent_MessageBuffer::ForwardComponent_MessageBuffer(Mercury::NetworkInterface & networkInterface) :
	Task(),
	networkInterface_(networkInterface),
	start_(false)
{
	// dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
ForwardComponent_MessageBuffer::~ForwardComponent_MessageBuffer()
{
	//dispatcher().cancelFrequentTask(this);
}

//-------------------------------------------------------------------------------------
Mercury::EventDispatcher & ForwardComponent_MessageBuffer::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
void ForwardComponent_MessageBuffer::push(COMPONENT_ID componentID, ForwardItem* pHandler)
{
	if(!start_)
	{
		dispatcher().addFrequentTask(this);
		start_ = true;
	}
	
	pMap_[componentID].push_back(pHandler);
}

//-------------------------------------------------------------------------------------
bool ForwardComponent_MessageBuffer::process()
{
	if(pMap_.size() <= 0)
	{
		start_ = false;
		return false;
	}
	
	MSGMAP::iterator iter = pMap_.begin();
	for(; iter != pMap_.end(); )
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(iter->first);
		if(cinfos == NULL || cinfos->pChannel == NULL)
			return true;

		if(iter->second.size() == 0)
		{
			pMap_.erase(iter++);
		}
		else
		{
			std::vector<ForwardItem*>::iterator itervec = iter->second.begin();

			for(; itervec != iter->second.end(); itervec++)
			{
				(*itervec)->pBundle->send(networkInterface_, cinfos->pChannel);
				Mercury::Bundle::ObjPool().reclaimObject((*itervec)->pBundle);
				(*itervec)->pBundle = NULL;

				if((*itervec)->pHandler != NULL)
				{
					(*itervec)->pHandler->process();
					SAFE_RELEASE((*itervec)->pHandler);
				}
				
				SAFE_RELEASE((*itervec));
			}
			
			DEBUG_MSG(fmt::format("ForwardComponent_MessageBuffer::process(): size:{}.\n", iter->second.size()));
			iter->second.clear();
			++iter;
		}
	}
	return true;
}



//-------------------------------------------------------------------------------------
ForwardAnywhere_MessageBuffer::ForwardAnywhere_MessageBuffer(Mercury::NetworkInterface & networkInterface, COMPONENT_TYPE forwardComponentType) :
	Task(),
	networkInterface_(networkInterface),
	forwardComponentType_(forwardComponentType),
	start_(false)
{
	// dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
ForwardAnywhere_MessageBuffer::~ForwardAnywhere_MessageBuffer()
{
	//dispatcher().cancelFrequentTask(this);
}

//-------------------------------------------------------------------------------------
Mercury::EventDispatcher & ForwardAnywhere_MessageBuffer::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
void ForwardAnywhere_MessageBuffer::push(ForwardItem* pHandler)
{
	if(!start_)
	{
		dispatcher().addFrequentTask(this);
		start_ = true;
	}
	
	pBundles_.push_back(pHandler);
}

//-------------------------------------------------------------------------------------
bool ForwardAnywhere_MessageBuffer::process()
{
	if(pBundles_.size() <= 0)
	{
		start_ = false;
		return false;
	}
	
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(forwardComponentType_);
	size_t idx = 0;

	if(cts.size() > 0)
	{
		// �������е����Ƶ���������ã� ���������ȴ���
		Components::COMPONENTS::iterator ctiter = cts.begin();
		for(; ctiter != cts.end(); ctiter++)
		{
			if((*ctiter).pChannel == NULL)
				return true;
		}

		std::vector<ForwardItem*>::iterator iter = pBundles_.begin();
		for(; iter != pBundles_.end(); iter++)
		{
			(*iter)->pBundle->send(networkInterface_, cts[idx].pChannel);
			Mercury::Bundle::ObjPool().reclaimObject((*iter)->pBundle);
			(*iter)->pBundle = NULL;

			idx++;
			if(idx >= cts.size())
				idx = 0;

			if((*iter)->pHandler != NULL)
			{
				(*iter)->pHandler->process();
				SAFE_RELEASE((*iter)->pHandler);
			}
			
			SAFE_RELEASE((*iter));
		}
		
		DEBUG_MSG(fmt::format("ForwardAnywhere_MessageBuffer::process(): size:{}.\n", pBundles_.size()));
		pBundles_.clear();
		start_ = false;
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------
}
