// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "db_interface_mysql.h"
#include "db_transaction.h"
#include "db_exception.h"
#include "db_interface/db_interface.h"
#include "helper/debug_helper.h"
#include "common/timestamp.h"
#include <mysql/mysqld_error.h>
#include <mysql/errmsg.h>

namespace KBEngine { 
namespace mysql {
 
static std::string SQL_START_TRANSACTION = "START TRANSACTION";
static std::string SQL_ROLLBACK = "ROLLBACK";
static std::string SQL_COMMIT = "COMMIT";

//-------------------------------------------------------------------------------------
DBTransaction::DBTransaction(DBInterface* pdbi, bool autostart):
	pdbi_(pdbi),
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
		pdbi_->query(SQL_START_TRANSACTION, false);
	}
	catch (DBException & e)
	{
		bool ret = static_cast<DBInterfaceMysql*>(pdbi_)->processException(e);
		KBE_ASSERT(ret);

		pdbi_->query(SQL_START_TRANSACTION, false);
	}

	static_cast<DBInterfaceMysql*>(pdbi_)->inTransaction(true);
}

//-------------------------------------------------------------------------------------
void DBTransaction::end()
{
	if (!committed_ && !static_cast<DBInterfaceMysql*>(pdbi_)->hasLostConnection())
	{
		try
		{
			WARNING_MSG( "DBTransaction::~DBTransaction: "
					"Rolling back\n" );

			pdbi_->query(SQL_ROLLBACK, false);
		}
		catch (DBException & e)
		{
			if (e.isLostConnection())
			{
				static_cast<DBInterfaceMysql*>(pdbi_)->hasLostConnection(true);
			}
		}
	}

	static_cast<DBInterfaceMysql*>(pdbi_)->inTransaction(false);
}

//-------------------------------------------------------------------------------------
bool DBTransaction::shouldRetry() const
{
	return (pdbi_->getlasterror() == ER_LOCK_DEADLOCK);
}

//-------------------------------------------------------------------------------------
void DBTransaction::commit()
{
	KBE_ASSERT(!committed_);

	uint64 startTime = timestamp();

	try
	{
		pdbi_->query(SQL_COMMIT, false);
	}
	catch (DBException & e)
	{
		bool ret = static_cast<DBInterfaceMysql*>(pdbi_)->processException(e);
		KBE_ASSERT(ret);
	}

	uint64 duration = timestamp() - startTime;
	if(duration > stampsPerSecond() * 0.2f)
	{
		WARNING_MSG(fmt::format("DBTransaction::commit(): took {:.2f} seconds\n", 
			(double(duration)/stampsPerSecondD())));
	}

	committed_ = true;
}

}
}
