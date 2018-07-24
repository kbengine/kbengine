// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_REDIS_EXCEPTION_H
#define KBE_REDIS_EXCEPTION_H

#include <string>

namespace KBEngine { 

class DBInterface;
class DBException : public std::exception
{
public:
	DBException(DBInterface* pdbi);
	~DBException() throw();

	virtual const char * what() const throw() { return errStr_.c_str(); }

	bool shouldRetry() const;
	bool isLostConnection() const;

private:
	std::string errStr_;
	unsigned int errNum_;
};

}

#endif // KBE_DB_EXCEPTION_H


