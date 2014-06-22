#include "TimeUtil.h"
#include "cocos2d.h"
USING_NS_CC;

NS_GC_BEGIN

TimeUtil::TimeUtil()
	:m_serverTime(0)
	,m_clientTime(0)
	,m_lastTimerFlag(0)
	,m_timerModify(0)
{

}
TimeUtil::~TimeUtil()
{
}
long TimeUtil::getTimer()
{
	struct cc_timeval now;
	CCTime::gettimeofdayCocos2d(&now, NULL);
	return (now.tv_sec * 1000 + now.tv_usec / 1000);
}
void TimeUtil::setServerTime(uint32 serverTime)
{
	this->m_serverTime = serverTime;
	m_clientTime = getTimer();
}
uint32 TimeUtil::timerFlag()
{
	uint32 _local1 = (uint32)(m_serverTime + (getTimer() - m_clientTime));
	m_timerModify = (_local1 - m_lastTimerFlag);
	m_lastTimerFlag = _local1;
	return (_local1);
}
int TimeUtil::currentTimestamp1()
{
	return (int)(time(NULL)*0.001);
}
float TimeUtil::currentTimestamp()
{
	return (time(NULL));
}
/*
std::string TimeUtil::getHourMinWithOutString(int _arg1)
{
	var _local2:int = int(((_arg1 / 60) % 60));
	var _local3:int = int((_arg1 / 3600));
	var _local4:String = (((((_local3 == 0)) && ((_local2 == 0)))) ? "00" : ((((_local2 < 10)) ? ("0" + _local2) : _local2) + ""));
	var _local5:String = (((_local3 == 0)) ? "00 " : ((((_local3 < 10)) ? ("0" + _local3) : _local3) + " "));
	return ((_local5 + _local4));
}

std::string TimeUtil::getDateBySecond(int _arg1)
{
	var _local2:Date = new Date((_arg1 * 1000));
	return ((((((((((_local2.getFullYear() + ResMgr.i().s("general_text_0006")) + (_local2.getMonth() + 1)) + ResMgr.i().s("general_text_0014")) + _local2.getDate()) + ResMgr.i().s("general_text_0015")) + _local2.getHours()) + ResMgr.i().s("general_text_0016")) + _local2.getMinutes()) + ResMgr.i().s("general_text_0004")));
}

std::string TimeUtil::time2DayString(int _arg1)
{
	var _local2 = "";
	var _local3:int;
	var _local4:int = (_arg1 / 3600);
	_local3 = (_local4 / 24);
	_local4 = (_local4 % 24);
	_local2 = (_local3.toString() + ResMgr.i().s("general_text_0002"));
	_local2 = (_local2 + (_local4.toString() + ResMgr.i().s("general_text_0003")));
	return (_local2);
}

std::string TimeUtil::getHourMinString(int _arg1)
{
	var _local2:int = int(((_arg1 / 60) % 60));
	var _local3:int = int((_arg1 / 3600));
	var _local4:String = (((((_local3 == 0)) && ((_local2 == 0)))) ? "00" : ((((_local2 < 10)) ? ("0" + _local2) : _local2) + ""));
	var _local5:String = (((_local3 == 0)) ? "00:" : ((((_local3 < 10)) ? ("0" + _local3) : _local3) + ":"));
	return ((_local5 + _local4));
}

std::string TimeUtil::data3String(int _arg1)
{
	var _local2 = "";
	var _local3:int;
	var _local4:int = (_arg1 / 3600);
	var _local5:int = ((_arg1 % 3600) / 60);
	var _local6:int = ((_arg1 % 3600) % 60);
	_local3 = (_local4 / 24);
	_local4 = (_local4 % 24);
	if (_local3 != 0)
	{
		_local2 = (_local3.toString() + ResMgr.i().s("general_text_0002"));
	};
	if (((!((_local4 == 0))) || (!((_local3 == 0)))))
	{
		_local2 = (_local2 + (_local4.toString() + ResMgr.i().s("general_text_0003")));
	};
	if (_local5 != 0)
	{
		_local2 = (_local2 + (_local5.toString() + ResMgr.i().s("general_text_0004")));
	};
	return (_local2);
}

std::string TimeUtil::getTimeString(int _arg1, int _arg2)
{
	if (_arg2 == 1)
	{
		return (getFormatTime(_arg1));
	};
	var _local3:int = (_arg1 % 60);
	var _local4:int = int(((_arg1 / 60) % 60));
	var _local5:int = int((_arg1 / 3600));
	var _local6:String = (((((((_local5 == 0)) && ((_local4 == 0)))) && ((_local3 == 0)))) ? "00" : ((((_local3 < 10)) ? ("0" + _local3) : _local3) + ""));
	var _local7:String = (((((_local5 == 0)) && ((_local4 == 0)))) ? "00:" : ((((_local4 < 10)) ? ("0" + _local4) : _local4) + ":"));
	var _local8:String = (((_local5 == 0)) ? "00:" : ((((_local5 < 10)) ? ("0" + _local5) : _local5) + ":"));
	return (((_local8 + _local7) + _local6));
}

std::string TimeUtil::replenishZero(std::string _arg1)
{
	if (_arg1.length == 1)
	{
		_arg1 = ("0" + _arg1);
	};
	return (_arg1);
}

std::string TimeUtil::data2String(int _arg1)
{
	var _local2 = "";
	var _local3:int;
	var _local4:int = (_arg1 / 3600);
	var _local5:int = ((_arg1 % 3600) / 60);
	var _local6:int = ((_arg1 % 3600) % 60);
	_local3 = (_local4 / 24);
	_local4 = (_local4 % 24);
	if (_local3 != 0)
	{
		_local2 = (_local3.toString() + ResMgr.i().s("general_text_0002"));
	};
	if (((!((_local4 == 0))) || (!((_local3 == 0)))))
	{
		_local2 = (_local2 + (_local4.toString() + ResMgr.i().s("general_text_0003")));
	};
	if (_local5 != 0)
	{
		_local2 = (_local2 + (_local5.toString() + ResMgr.i().s("general_text_0004")));
	};
	_local2 = (_local2 + (_local6.toString() + ResMgr.i().s("general_text_0001")));
	return (_local2);
}

std::string TimeUtil::getFormatTime(int _arg1)
{
	var _local2:int = (_arg1 % 60);
	var _local3:int = int(((_arg1 / 60) % 60));
	var _local4:int = int((_arg1 / 3600));
	var _local5:String = (((((((_local4 == 0)) && ((_local3 == 0)))) && ((_local2 == 0)))) ? "" : ((((_local2 < 10)) ? ("0" + _local2) : _local2) + ResMgr.i().s("general_text_0001")));
	var _local6:String = (((((_local4 == 0)) && ((_local3 == 0)))) ? "" : ((((_local3 < 10)) ? ("0" + _local3) : _local3) + ResMgr.i().s("general_text_0004")));
	var _local7:String = (((_local4 == 0)) ? "" : ((((_local4 < 10)) ? ("0" + _local4) : _local4) + ResMgr.i().s("general_text_0003")));
	return (((_local7 + _local6) + _local5));
}

float TimeUtil::currentTimestamp()
{
	return (Math.round((new Date().getTime() * 0.001)));
}*/

NS_GC_END