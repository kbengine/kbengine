// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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

