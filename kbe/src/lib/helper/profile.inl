// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

namespace KBEngine
{

INLINE const char* ProfileGroup::name() const
{ 
	return name_.c_str(); 
}

INLINE const ProfileGroup::PROFILEVALS& ProfileGroup::profiles() const
{
	return profiles_;
}

INLINE const char * ProfileVal::c_str() const { return name_.c_str(); }

INLINE double ProfileVal::lastTimeInSeconds() const { return stampsToSeconds(lastTime_); }
INLINE double ProfileVal::sumTimeInSeconds() const  { return stampsToSeconds( sumTime_ ); }
INLINE double ProfileVal::lastIntTimeInSeconds() const { return stampsToSeconds( lastIntTime_ ); }
INLINE double ProfileVal::sumIntTimeInSeconds() const { return stampsToSeconds( sumIntTime_ ); }

INLINE TimeStamp ProfileVal::lastTime() const
{
	return this->running() ? TimeStamp( 0 ) : lastTime_;
}

INLINE TimeStamp ProfileVal::sumTime() const
{
	return sumTime_;
}

INLINE TimeStamp ProfileVal::lastIntTime() const
{
	return lastIntTime_;
}

INLINE TimeStamp ProfileVal::sumIntTime() const
{
	return sumIntTime_;
}
	
INLINE bool ProfileVal::running() const
{
	return inProgress_ > 0;
}

INLINE bool ProfileVal::stop(const char * filename, int lineNum, uint32 qty)
{
	this->stop(qty);

	const bool tooLong = isTooLong();

	if (tooLong)
	{
		WARNING_MSG(fmt::format("{}:{}: Profile {} took {:.2f} seconds\n",
			filename,
			lineNum,
			name_.c_str(),
			(lastTime_  / stampsPerSecondD())));
	}

	return true;
}

INLINE bool ProfileVal::isTooLong() const
{
	return !this->running() && (lastTime_ > warningPeriod_);
}

INLINE const char* ProfileVal::name() const
{ 
	return name_.c_str(); 
}

INLINE uint32 ProfileVal::count() const
{
	return count_;
}

}



