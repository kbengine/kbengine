#pragma once

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