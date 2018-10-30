// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBENGINE_PROFILE_H
#define KBENGINE_PROFILE_H

#include "debug_helper.h"
#include "common/common.h"
#include "common/timer.h"
#include "common/timestamp.h"

namespace KBEngine
{

#if ENABLE_WATCHERS

class ProfileVal;

class ProfileGroup
{
public:
	ProfileGroup(std::string name = "default");
	~ProfileGroup();

	typedef std::vector<ProfileVal*> PROFILEVALS;
	typedef PROFILEVALS::iterator iterator;

	static ProfileGroup & defaultGroup();

	PROFILEVALS & stack() { return stack_; }
	void add(ProfileVal * pVal);

	iterator begin() { return profiles_.begin(); }
	iterator end() { return profiles_.end(); }

	ProfileVal * pRunningTime() { return profiles_[0]; }
	const ProfileVal * pRunningTime() const { return profiles_[0]; }
	TimeStamp runningTime() const;

	INLINE const char* name() const;

	bool initializeWatcher();

	static void finalise(void);

	INLINE const ProfileGroup::PROFILEVALS& profiles() const;

private:
	PROFILEVALS profiles_;
	PROFILEVALS stack_;
	std::string name_;
};

class ProfileVal
{
public:
	ProfileVal(std::string name, ProfileGroup * pGroup = NULL);
	~ProfileVal();

	bool initializeWatcher();

	void start()
	{
		if(!initWatcher_ && count_ > 10)
			initializeWatcher();

		TimeStamp now = timestamp();

		// 记录第几次处理
		if (inProgress_++ == 0)
			lastTime_ = now;

		ProfileGroup::PROFILEVALS & stack = pProfileGroup_->stack();

		// 如果栈中有对象则自己是从上一个ProfileVal函数进入调用的
		// 我们可以在此得到上一个函数进入到本函数之前的一段时间片
		// 然后将其加入到sumIntTime_
		if (!stack.empty()){
			ProfileVal & profile = *stack.back();
			profile.lastIntTime_ = now - profile.lastIntTime_;
			profile.sumIntTime_ += profile.lastIntTime_;
		}

		// 将自己压栈
		stack.push_back(this);

		// 记录开始时间
		lastIntTime_ = now;
	}

	void stop(uint32 qty = 0)
	{
		TimeStamp now = timestamp();

		// 如果为0则表明自己是调用栈的产生着
		// 在此我们可以得到这个函数总共耗费的时间
		if (--inProgress_ == 0){
			lastTime_ = now - lastTime_;
			sumTime_ += lastTime_;
		}

		lastQuantity_ = qty;
		sumQuantity_ += qty;
		++count_;

		ProfileGroup::PROFILEVALS & stack = pProfileGroup_->stack();
		KBE_ASSERT( stack.back() == this );

		stack.pop_back();

		// 得到本函数所耗费的时间
		lastIntTime_ = now - lastIntTime_;
		sumIntTime_ += lastIntTime_;

		// 我们需要在此重设上一个函数中的profile对象的最后一次内部时间
		// 使其能够在start时正确得到自调用完本函数之后进入下一个profile函数中时所消耗
		// 的时间片段
		if (!stack.empty())
			stack.back()->lastIntTime_ = now;
	}

	INLINE bool stop( const char * filename, int lineNum, uint32 qty = 0);

	INLINE bool running() const;

	INLINE const char * c_str() const;

	INLINE TimeStamp sumTime() const;
	INLINE TimeStamp lastIntTime() const ;
	INLINE TimeStamp sumIntTime() const ;
	INLINE TimeStamp lastTime() const;

	INLINE double lastTimeInSeconds() const;
	INLINE double sumTimeInSeconds() const ;
	INLINE double lastIntTimeInSeconds() const ;
	INLINE double sumIntTimeInSeconds() const;

	INLINE const char* name() const;

	INLINE uint32 count() const;

	INLINE bool isTooLong() const;

	static void setWarningPeriod(TimeStamp warningPeriod) { warningPeriod_ = warningPeriod; }

	// 名称
	std::string		name_;

	// ProfileGroup指针
	ProfileGroup * pProfileGroup_;

	// startd后的时间.
	TimeStamp		lastTime_;

	// count_次的总时间
	TimeStamp		sumTime_;

	// 记录最后一次内部时间片
	TimeStamp		lastIntTime_;

	// count_次内部总时间
	TimeStamp		sumIntTime_;

	uint32			lastQuantity_;	///< The last value passed into stop.
	uint32			sumQuantity_;	///< The total of all values passed into stop.
	uint32			count_;			///< The number of times stop has been called.

	// 记录第几次处理, 如递归等
	int				inProgress_;

	bool			initWatcher_;

private:
	static TimeStamp warningPeriod_;

};

class ScopedProfile
{
public:
	ScopedProfile(ProfileVal & profile, const char * filename, int lineNum) :
		profile_(profile),
		filename_(filename),
		lineNum_(lineNum)
	{
		profile_.start();
	}

	~ScopedProfile()
	{
		profile_.stop(filename_, lineNum_);
	}

private:
	ProfileVal& profile_;
	const char* filename_;
	int lineNum_;

};

#define START_PROFILE( PROFILE ) PROFILE.start();

#define STOP_PROFILE( PROFILE )	PROFILE.stop( __FILE__, __LINE__ );

#define AUTO_SCOPED_PROFILE( NAME )												\
	static ProfileVal _localProfile( NAME );									\
	ScopedProfile _autoScopedProfile( _localProfile, __FILE__, __LINE__ );

#define SCOPED_PROFILE(PROFILE)													\
	ScopedProfile PROFILE##_scopedProfile(PROFILE, __FILE__, __LINE__);

#define STOP_PROFILE_WITH_CHECK( PROFILE )										\
	if (PROFILE.stop( __FILE__, __LINE__ ))

#define STOP_PROFILE_WITH_DATA( PROFILE, DATA )									\
	PROFILE.stop( __FILE__, __LINE__ , DATA );

// 由此可得到系统profile时间
uint64 runningTime();

#else

#define AUTO_SCOPED_PROFILE( NAME )
#define STOP_PROFILE_WITH_DATA( PROFILE, DATA )
#define STOP_PROFILE_WITH_CHECK( PROFILE )
#define SCOPED_PROFILE(PROFILE)
#define STOP_PROFILE( PROFILE )
#define START_PROFILE( PROFILE )

uint64 runningTime(){
	return 0;
}

#endif //ENABLE_WATCHERS


}

#ifdef CODE_INLINE
#include "profile.inl"
#endif

#endif // KBENGINE_PROFILE_H


