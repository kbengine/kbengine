// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_LOGWATCHER_H
#define KBE_LOGWATCHER_H
	
// common include	

//#define NDEBUG
#include "common/common.h"
#include "network/address.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine
{
class MemoryStream;
struct LOG_ITEM;

struct FilterOptions
{
	int32 uid;
	uint32 logtypes;
	uint8 componentBitmap[COMPONENT_END_TYPE];
	COMPONENT_ORDER globalOrder;
	COMPONENT_ORDER groupOrder;
	std::string keyStr;
	std::string date;
};

class LogWatcher
{
public:
	enum STATES
	{
		STATE_AUTO = 0,
		STATE_FINDING = 1,
	};

	LogWatcher();
	~LogWatcher();

	bool createFromStream(MemoryStream * s);
	bool updateSetting(MemoryStream * s);

	void reset();
	void addr(const Network::Address& address) { addr_ = address; }
	
	void onMessage(LOG_ITEM* pLogItem);

	STATES state() const{ return state_; }

protected:
	bool validDate_(const std::string& log);
	bool containKeyworlds_(const std::string& log);

protected:
	Network::Address addr_;
	FilterOptions filterOptions_;
	STATES state_;
};

}

#endif // KBE_LOGWATCHER_H
