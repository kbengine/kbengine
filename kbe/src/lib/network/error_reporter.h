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

#ifndef KBE_NETWORKERROR_REPORTER_H
#define KBE_NETWORKERROR_REPORTER_H

#include "common/common.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/network_exception.h"

namespace KBEngine { 
namespace Network
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
	ErrorReporter(EventDispatcher & dispatcher);
	~ErrorReporter();

	void reportException(Reason reason, const Address & addr = Address::NONE,
			const char * prefix = NULL);
	void reportPendingExceptions(bool reportBelowThreshold = false);

private:
	void reportException(const NetworkException & ne, const char * prefix = NULL);

	void reportError(const Address & address, const char* format, ...);


	static const uint ERROR_REPORT_MIN_PERIOD_MS;
	static const uint ERROR_REPORT_COUNT_MAX_LIFETIME_MS;

	void addReport(const Address & address, const std::string & error);

	static std::string addressErrorToString(
			const Address & address,
			const std::string & errorString);

	static std::string addressErrorToString(
			const Address & address,
			const std::string & errorString,
			const ErrorReportAndCount & reportAndCount,
			const uint64 & now);

	virtual void handleTimeout(TimerHandle handle, void * arg);

	TimerHandle reportLimitTimerHandle_;
	ErrorsAndCounts errorsAndCounts_;
};


}
}

#ifdef CODE_INLINE
#include "error_reporter.inl"
#endif
#endif // KBE_NETWORKERROR_REPORTER_H
