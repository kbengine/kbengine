// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "error_reporter.h"
#ifndef CODE_INLINE
#include "error_reporter.inl"
#endif

#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/endpoint.h"

namespace KBEngine { 
namespace Network
{

const uint ErrorReporter::ERROR_REPORT_MIN_PERIOD_MS = 2000;
const uint ErrorReporter::ERROR_REPORT_COUNT_MAX_LIFETIME_MS = 10000;

//-------------------------------------------------------------------------------------
ErrorReporter::ErrorReporter(EventDispatcher & dispatcher) :
	reportLimitTimerHandle_(),
	errorsAndCounts_()
{
	reportLimitTimerHandle_ = dispatcher.addTimer(
			ERROR_REPORT_MIN_PERIOD_MS * 1000, this);
}

//-------------------------------------------------------------------------------------
ErrorReporter::~ErrorReporter()
{
	reportLimitTimerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
std::string ErrorReporter::addressErrorToString(const Address & address,
		const std::string & errorString)
{
	std::ostringstream out;
	out << address.c_str() << ": " << errorString;
	return out.str();
}

//-------------------------------------------------------------------------------------
std::string ErrorReporter::addressErrorToString(
		const Address & address,
		const std::string & errorString,
		const ErrorReportAndCount & reportAndCount,
		const uint64 & now)
{
	int64 deltaStamps = now - reportAndCount.lastReportStamps;
	double deltaMillis = 1000 * deltaStamps / stampsPerSecondD();

	char * buf = NULL;
	int bufLen = 64;
	int strLen = bufLen;
	do
	{
		bufLen = strLen + 1;
		delete [] buf;
		buf = new char[ bufLen ];

#if KBE_PLATFORM == PLATFORM_WIN32
		strLen = _snprintf(buf, bufLen, "%d reports of '%s' "
				"in the last %.00fms",
			reportAndCount.count,
			addressErrorToString(address, errorString).c_str(),
			deltaMillis);
		if (strLen == -1) strLen = (bufLen - 1) * 2;
#else
		strLen = snprintf(buf, bufLen, "%d reports of '%s' "
				"in the last %.00fms",
			reportAndCount.count,
			addressErrorToString(address, errorString).c_str(),
			deltaMillis);
#endif
	} while (strLen >= bufLen);

	std::string out(buf);
	delete [] buf;
	return out;
}

//-------------------------------------------------------------------------------------
void ErrorReporter::reportError(
		const Address & address, const char* format, ...)
{
	char * buf = NULL;
	int bufLen = 32;
	int strLen = bufLen;
	do
	{
		delete [] buf;
		bufLen = strLen + 1;
		buf = new char[ bufLen ];

		va_list va;
		va_start(va, format);

#if KBE_PLATFORM == PLATFORM_WIN32
		strLen = _vsnprintf(buf, bufLen, format, va);
		if (strLen == -1) strLen = (bufLen - 1) * 2;
#else
		strLen = vsnprintf(buf, bufLen, format, va);
#endif
		va_end(va);
		buf[bufLen -1] = '\0';

	} while (strLen >= bufLen);

	std::string error(buf);

	delete [] buf;

	this->addReport(address, error);
}

//-------------------------------------------------------------------------------------
void ErrorReporter::reportException(Reason reason,
		const Address & addr,
		const char* prefix,
		const char* suffix)
{
	NetworkException ne(reason, addr);
	this->reportException(ne, prefix, suffix);

}

//-------------------------------------------------------------------------------------
void ErrorReporter::reportException(const NetworkException & ne, const char* prefix, const char* suffix)
{
	Address offender(0, 0);
	ne.getAddress(offender);

	if (prefix)
	{
		if (!suffix)
		{
			this->reportError(offender,
				"%s: Exception occurred: %s",
				prefix, reasonToString(ne.reason()));
		}
		else
		{
			this->reportError(offender,
				"%s: Exception occurred: %s %s",
				prefix, reasonToString(ne.reason()), suffix);
		}
	}
	else
	{
		if (!suffix)
		{
			this->reportError(offender, "Exception occurred: %s",
				reasonToString(ne.reason()));
		}
		else
		{
			this->reportError(offender, "Exception occurred: %s %s",
				reasonToString(ne.reason()), suffix);
		}
	}
}

//-------------------------------------------------------------------------------------
void ErrorReporter::addReport(const Address & address, const std::string & errorString)
{
	AddressAndErrorString addressError(address, errorString);
	ErrorsAndCounts::iterator searchIter =
		errorsAndCounts_.find(addressError);

	uint64 now = timestamp();

	if (searchIter != errorsAndCounts_.end())
	{
		ErrorReportAndCount & reportAndCount = searchIter->second;
		reportAndCount.count++;

		int64 millisSinceLastReport = 1000 *
			(now - reportAndCount.lastReportStamps) /
			stampsPerSecond();

		reportAndCount.lastRaisedStamps = now;

		if (millisSinceLastReport >= ERROR_REPORT_MIN_PERIOD_MS)
		{
			ERROR_MSG(fmt::format("{}\n", addressErrorToString(address, errorString,
					reportAndCount, now).c_str()));

			reportAndCount.count = 0;
			reportAndCount.lastReportStamps = now;
		}

	}
	else
	{
		ERROR_MSG(fmt::format("{}\n", addressErrorToString(address, errorString).c_str()));

		ErrorReportAndCount reportAndCount = { now, now, 0, };
		errorsAndCounts_[ addressError ] = reportAndCount;
	}
}

//-------------------------------------------------------------------------------------
void ErrorReporter::reportPendingExceptions(bool reportBelowThreshold)
{
	uint64 now = timestamp();

	ErrorsAndCounts::iterator staleIter = this->errorsAndCounts_.end();

	for (	ErrorsAndCounts::iterator exceptionCountIter =
				this->errorsAndCounts_.begin();
			exceptionCountIter != this->errorsAndCounts_.end();
			++exceptionCountIter)
	{
		if (staleIter != this->errorsAndCounts_.end())
		{
			this->errorsAndCounts_.erase(staleIter);
			staleIter = this->errorsAndCounts_.end();
		}

		const AddressAndErrorString & addressError =
			exceptionCountIter->first;
		ErrorReportAndCount & reportAndCount = exceptionCountIter->second;

		int64 millisSinceLastReport = 1000 *
			(now - reportAndCount.lastReportStamps) / stampsPerSecond();
		if (reportBelowThreshold ||
				millisSinceLastReport >= ERROR_REPORT_MIN_PERIOD_MS)
		{
			if (reportAndCount.count)
			{
				ERROR_MSG(fmt::format("{}\n",
					addressErrorToString(
						addressError.first, addressError.second,
						reportAndCount, now).c_str()
				));
				reportAndCount.count = 0;
				reportAndCount.lastReportStamps = now;

			}
		}

		uint64 sinceLastRaisedMillis = 1000 * (now - reportAndCount.lastRaisedStamps) /
			stampsPerSecond();
		if (sinceLastRaisedMillis > ERROR_REPORT_COUNT_MAX_LIFETIME_MS)
		{
			staleIter = exceptionCountIter;
		}
	}

	if (staleIter != this->errorsAndCounts_.end())
	{
		this->errorsAndCounts_.erase(staleIter);
	}
}

//-------------------------------------------------------------------------------------
void ErrorReporter::handleTimeout(TimerHandle handle, void * arg)
{
	KBE_ASSERT(handle == reportLimitTimerHandle_);
	this->reportPendingExceptions();
}

}
}
