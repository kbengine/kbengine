/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_INTERFACES_ORDERS_H
#define KBE_INTERFACES_ORDERS_H

#include "common/common.h"
#include "common/memorystream.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"
#include "network/address.h"
#include "network/endpoint.h"

namespace KBEngine{ 

class Orders
{
public:
	enum State
	{
		STATE_FAILED = 0,
		STATE_SUCCESS = 1
	};
	
	Orders();
	virtual ~Orders();
	
	Network::Address address;
	std::string ordersID;
	CALLBACK_ID cbid;
	DBID dbid;
	COMPONENT_ID baseappID;
	COMPONENT_ID dbmgrID;
	State state;
	
	std::string postDatas;
	std::string getDatas;

	uint64 timeout;
};

class OrdersCharge : public Orders
{
public:
	OrdersCharge();
	virtual ~OrdersCharge();
};

}

#endif // KBE_INTERFACES_ORDERS_H
