// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "db_exception.h"
#include "db_interface_redis.h"
#include "db_interface/db_interface.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
DBException::DBException(DBInterface* pdbi) :
			errStr_(static_cast<DBInterfaceRedis*>(pdbi)->getstrerror()),
			errNum_(static_cast<DBInterfaceRedis*>(pdbi)->getlasterror())
{
}

//-------------------------------------------------------------------------------------
DBException::~DBException() throw()
{
}

//-------------------------------------------------------------------------------------
bool DBException::shouldRetry() const
{
	return (errNum_== REDIS_ERR_OOM) ||
			(errNum_ == REDIS_ERR_OTHER);
}

//-------------------------------------------------------------------------------------
bool DBException::isLostConnection() const
{
	return (errNum_ == REDIS_ERR_IO) ||
			(errNum_ == REDIS_ERR_EOF);
}

//-------------------------------------------------------------------------------------
}

// db_exception.cpp
