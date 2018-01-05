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

#ifndef KBE_REDIS_TRANSACTION_HELPER_H
#define KBE_REDIS_TRANSACTION_HELPER_H

#include "hiredis.h"

namespace KBEngine { 
class DBInterface;
namespace redis {

/**
 */
class DBTransaction
{
public:
	DBTransaction(DBInterface* pdbi, bool autostart = true);
	~DBTransaction();
	
	void start();
	void end();

	void commit();
	void rollback();
	
	bool shouldRetry() const;

	void pdbi(DBInterface* pdbi){ pdbi_ = pdbi; }
	
	redisReply* pRedisReply(){ return pRedisReply_; }

private:
	DBInterface* pdbi_;
	bool committed_;
	bool autostart_;
	redisReply* pRedisReply_;
};

}
}
#endif // KBE_REDIS_TRANSACTION_HELPER_H

