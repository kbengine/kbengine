// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_MYSQL_TRANSACTION_HELPER_H
#define KBE_MYSQL_TRANSACTION_HELPER_H

namespace KBEngine { 
class DBInterface;
namespace mysql {

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

	bool shouldRetry() const;

	void pdbi(DBInterface* pdbi){ pdbi_ = pdbi; }
	
private:
	DBInterface* pdbi_;
	bool committed_;
	bool autostart_;
};

}
}
#endif // KBE_MYSQL_TRANSACTION_HELPER_H

