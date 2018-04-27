// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "network/bundle.h"
#include "interfaces.h"
#include "orders.h"


namespace KBEngine{

//-------------------------------------------------------------------------------------
Orders::Orders():
address(),
ordersID(),
cbid(0),
dbid(0),
baseappID(UNKNOWN_COMPONENT_TYPE),
dbmgrID(UNKNOWN_COMPONENT_TYPE),
state(Orders::STATE_FAILED),
postDatas(),
getDatas(),
timeout(0)
{
}

//-------------------------------------------------------------------------------------
Orders::~Orders()
{
}

//-------------------------------------------------------------------------------------
OrdersCharge::OrdersCharge()
{
}

//-------------------------------------------------------------------------------------
OrdersCharge::~OrdersCharge()
{
	//INFO_MSG(fmt::format("OrdersCharge::~OrdersCharge({0:p}): orders={1}\n", 
	//	(void*)this, ordersID));
}

//-------------------------------------------------------------------------------------
}
