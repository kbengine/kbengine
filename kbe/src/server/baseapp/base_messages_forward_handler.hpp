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

#ifndef KBE_BASE_MESSAGES_FORWARD_HANDLER_HPP
#define KBE_BASE_MESSAGES_FORWARD_HANDLER_HPP

#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"

namespace KBEngine{

class Base;
class BaseMessagesForwardHandler : public Task
{
public:
	BaseMessagesForwardHandler(Base* pBase);
	~BaseMessagesForwardHandler();
	
	bool process();

	void pushMessages(Mercury::Bundle* pBundle);

	void startForward(){ startForward_ = true; }
	void stopForward(){ startForward_ = false; }
private:
	Base* pBase_;
	bool completed_;
	std::vector<Mercury::Bundle*> bufferedSendToCellappMessages_;
	bool startForward_;
};


}

#endif // KBE_BASE_MESSAGES_FORWARD_HANDLER_HPP
