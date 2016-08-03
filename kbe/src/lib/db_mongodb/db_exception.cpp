#include "db_exception.h"
#include "db_interface_mongodb.h"
#include "db_interface/db_interface.h"

namespace KBEngine {

	//-------------------------------------------------------------------------------------
	DBException::DBException(DBInterface* pdbi) :
		errStr_(static_cast<DBInterfaceMongodb*>(pdbi)->getstrerror()),
		errNum_(static_cast<DBInterfaceMongodb*>(pdbi)->getlasterror())
	{
	}

	//-------------------------------------------------------------------------------------
	DBException::~DBException() throw()
	{
	}

	//-------------------------------------------------------------------------------------
	bool DBException::shouldRetry() const
	{
		/*return (errNum_ == REDIS_ERR_OOM) ||
			(errNum_ == REDIS_ERR_OTHER);*/
		return true;
	}

	//-------------------------------------------------------------------------------------
	bool DBException::isLostConnection() const
	{
		/*return (errNum_ == REDIS_ERR_IO) ||
			(errNum_ == REDIS_ERR_EOF);*/
		return true;
	}

	//-------------------------------------------------------------------------------------
}