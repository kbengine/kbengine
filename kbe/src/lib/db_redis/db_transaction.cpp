// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "db_interface_redis.h"
#include "db_transaction.h"
#include "db_exception.h"
#include "db_interface/db_interface.h"
#include "helper/debug_helper.h"
#include "common/timestamp.h"

namespace KBEngine { 
namespace redis {

static std::string SQL_START_TRANSACTION = "MULTI";
static std::string SQL_ROLLBACK = "DISCARD";
static std::string SQL_COMMIT = "EXEC";

//-------------------------------------------------------------------------------------
DBTransaction::DBTransaction(DBInterface* pdbi, bool autostart):
	pdbi_(pdbi),
	committed_(false),
	autostart_(autostart),
	pRedisReply_(NULL)
{
	if(autostart)
		start();
}

//-------------------------------------------------------------------------------------
DBTransaction::~DBTransaction()
{
	if(autostart_)
		end();
	
	if(pRedisReply_){
		freeReplyObject(pRedisReply_); 
		pRedisReply_ = NULL;
	}
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
		bool ret = static_cast<DBInterfaceRedis*>(pdbi_)->processException(e);
		KBE_ASSERT(ret);
	}

	static_cast<DBInterfaceRedis*>(pdbi_)->inTransaction(true);
}

//-------------------------------------------------------------------------------------
void DBTransaction::rollback()
{
	if(committed_)
		return;
	
	WARNING_MSG( "DBTransaction::rollback: "
			"Rolling back\n" );

	pdbi_->query(SQL_ROLLBACK, false);	
}

//-------------------------------------------------------------------------------------
void DBTransaction::end()
{
	if(!committed_ && !static_cast<DBInterfaceRedis*>(pdbi_)->hasLostConnection())
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
				static_cast<DBInterfaceRedis*>(pdbi_)->hasLostConnection(true);
			}
		}
	}

	static_cast<DBInterfaceRedis*>(pdbi_)->inTransaction(false);
}

//-------------------------------------------------------------------------------------
bool DBTransaction::shouldRetry() const
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTransaction::commit()
{
	KBE_ASSERT(!committed_);

	uint64 startTime = timestamp();
	static_cast<DBInterfaceRedis*>(pdbi_)->query(SQL_COMMIT, &pRedisReply_, false);

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
