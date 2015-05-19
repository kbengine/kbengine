/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#include "db_exception.h"
#include "db_interface_mysql.h"
#include "db_interface/db_interface.h"
#include <mysql/mysqld_error.h>
#include <mysql/errmsg.h>

namespace KBEngine { 

//-------------------------------------------------------------------------------------
DBException::DBException(DBInterface* dbi) :
			errStr_(mysql_error(static_cast<DBInterfaceMysql*>(dbi)->mysql())),
			errNum_(mysql_errno(static_cast<DBInterfaceMysql*>(dbi)->mysql()))
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
