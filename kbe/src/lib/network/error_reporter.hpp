/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORKERROR_REPORTER__
#define __NETWORKERROR_REPORTER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/nub_exception.hpp"

namespace KBEngine { 
namespace Mercury
{
class EventDispatcher;
class Address;

struct ErrorReportAndCount
{
	uint64 lastReportStamps;	//< When this error was last reported
	uint64 lastRaisedStamps;	//< When this error was last raised
	uint count;					//< How many of this exception have been
								//	reported since
};

typedef std::pair< Address, std::string > AddressAndErrorString;

typedef std::map< AddressAndErrorString, ErrorReportAndCount >
	ErrorsAndCounts;

class ErrorReporter : public TimerHandler
{
public:
	ErrorReporter( EventDispatcher & dispatcher );
	~ErrorReporter();

	void reportException( Reason reason, const Address & addr = Address::NONE,
			const char * prefix = NULL );
	void reportPendingExceptions( bool reportBelowThreshold = false );

private:
	void reportException( const NubException & ne, const char * prefix = NULL );

	void reportError( const Address & address, const char* format, ... );


	static const uint ERROR_REPORT_MIN_PERIOD_MS;
	static const uint ERROR_REPORT_COUNT_MAX_LIFETIME_MS;

	void addReport( const Address & address, const std::string & error );

	static std::string addressErrorToString(
			const Address & address,
			const std::string & errorString );

	static std::string addressErrorToString(
			const Address & address,
			const std::string & errorString,
			const ErrorReportAndCount & reportAndCount,
			const uint64 & now );

	virtual void handleTimeout( TimerHandle handle, void * arg );

	TimerHandle reportLimitTimerHandle_;
	ErrorsAndCounts errorsAndCounts_;
};


}
}

#ifdef CODE_INLINE
#include "error_reporter.ipp"
#endif
#endif // __NETWORKERROR_REPORTER__