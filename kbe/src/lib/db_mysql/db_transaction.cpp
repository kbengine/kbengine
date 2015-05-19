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

#include "db_interface_mysql.h"
#include "db_transaction.h"
#include "db_exception.h"
#include "db_interface/db_interface.h"
#include "helper/debug_helper.h"
#include "common/timestamp.h"
#include <mysql/mysqld_error.h>
#include <mysql/errmsg.h>

namespace KBEngine { 

std::string SQL_START_TRANSACTION = "START TRANSACTION";
std::string SQL_ROLLBACK = "ROLLBACK";
std::string SQL_COMMIT = "COMMIT";

//-------------------------------------------------------------------------------------
DBTransaction::DBTransaction(DBInterface* dbi, bool autostart):
	dbi_(dbi),
	committed_(false),
	autostart_(autostart)
{
	if(autostart)
		start();
}

//-------------------------------------------------------------------------------------
DBTransaction::~DBTransaction()
{
	if(autostart_)
		end();
}

//-------------------------------------------------------------------------------------
void DBTransaction::start()
{
	committed_ = false;

	try
	{
		dbi_->query(SQL_START_TRANSACTION, false);
	}
	catch (DBException & e)
	{
		bool ret = static_cast<DBInterfaceMysql*>(dbi_)->processException(e);
		KBE_ASSERT(ret);
	}

	static_cast<DBInterfaceMysql*>(dbi_)->inTransaction(true);
}

//-------------------------------------------------------------------------------------
void DBTransaction::end()
{
	if (!committed_ && !static_cast<DBInterfaceMysql*>(dbi_)->hasLostConnection())
	{
		try
		{
			WARNING_MSG( "DBTransaction::~DBTransaction: "
					"Rolling back\n" );

			dbi_->query(SQL_ROLLBACK, false);
		}
		catch (DBException & e)
		{
			if (e.isLostConnection())
			{
				static_cast<DBInterfaceMysql*>(dbi_)->hasLostConnection(true);
			}
		}
	}

	static_cast<DBInterfaceMysql*>(dbi_)->inTransaction(false);
}

//-------------------------------------------------------------------------------------
bool DBTransaction::shouldRetry() const
{
	return (dbi_->getlasterror() == ER_LOCK_DEADLOCK);
}

//-------------------------------------------------------------------------------------
void DBTransaction::commit()
{
	KBE_ASSERT(!committed_);

	uint64 startTime = timestamp();
	dbi_->query(SQL_COMMIT, false);

	uint64 duration = timestamp() - startTime;
	if(duration > stampsPerSecond() * 0.2f)
	{
		WARNING_MSG(fmt::format("DBTransaction::commit(): took {:.2f} seconds\n", 
			(double(duration)/stampsPerSecondD())));
	}

	committed_ = true;
}

}

