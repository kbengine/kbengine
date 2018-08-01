// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "db_exception.h"
#include "db_interface_mysql.h"
#include "db_interface/db_interface.h"
#include <mysql/mysqld_error.h>
#include <mysql/errmsg.h>

namespace KBEngine { 

//-------------------------------------------------------------------------------------
DBException::DBException(DBInterface* pdbi) :
	errStr_(pdbi ? mysql_error(static_cast<DBInterfaceMysql*>(pdbi)->mysql()) : ""),
	errNum_(pdbi ? mysql_errno(static_cast<DBInterfaceMysql*>(pdbi)->mysql()) : 0)
{
}

//-------------------------------------------------------------------------------------
DBException::~DBException() throw()
{
}

//-------------------------------------------------------------------------------------
bool DBException::shouldRetry() const
{
	return (errNum_== ER_LOCK_DEADLOCK) ||
			(errNum_ == ER_LOCK_WAIT_TIMEOUT);
}

//-------------------------------------------------------------------------------------
bool DBException::isLostConnection() const
{
	return (errNum_ == CR_SERVER_GONE_ERROR) ||
			(errNum_ == CR_SERVER_LOST);
}

//-------------------------------------------------------------------------------------
}

// db_exception.cpp
