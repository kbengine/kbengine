// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_TIMESTAMP_H
#define KBE_TIMESTAMP_H

#include "common/platform.h"

namespace KBEngine {

// ָʾ�Ƿ����ͨ������RDTSC��ʱ�����������
// ����ʱ�����ʹ�ô˵ĺô��ǣ����ܿ��ٺ;�ȷ�ķ���ʵ�ʵ�ʱ�ӵδ�
// ������֮���ǣ��Ⲣ��ʹ��SpeedStep�������ı����ǵ�ʱ���ٶȵ�CPU��
#if KBE_PLATFORM == PLATFORM_UNIX
	//#define KBE_USE_RDTSC
#else // unix
	//#define KBE_USE_RDTSC
#endif // unix

	enum KBETimingMethod
	{
		RDTSC_TIMING_METHOD, // ��CPU�ϵ�������������ʱ��������,�ﵽ���뼶�ļ�ʱ����
		GET_TIME_OF_DAY_TIMING_METHOD,
		GET_TIME_TIMING_METHOD,
		NO_TIMING_METHOD,
	};

	extern KBETimingMethod g_timingMethod;

	const char* getTimingMethodName();

#if KBE_PLATFORM == PLATFORM_UNIX

	inline uint64 timestamp_rdtsc()
	{
		uint32 rethi, retlo;
		__asm__ __volatile__(
			"rdtsc":
		"=d"    (rethi),
			"=a"    (retlo)
			);
		return uint64(rethi) << 32 | retlo;
	}

	// ʹ�� gettimeofday. ���Դ�ű�RDTSC20��-600����
	// ���⣬��һ������
	// 2.4�ں��£��������ε���gettimeofday�Ŀ���
	// ����һ������ǵ����ߡ�
#include <sys/time.h>

	inline uint64 timestamp_gettimeofday()
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		return 1000000ULL * uint64(tv.tv_sec) + uint64(tv.tv_usec);
	}

#include <time.h>
#include <asm/unistd.h>

	inline uint64 timestamp_gettime()
	{
		timespec tv;
		assert(syscall(__NR_clock_gettime, CLOCK_MONOTONIC, &tv) == 0);
		return 1000000000ULL * tv.tv_sec + tv.tv_nsec;
	}

	inline uint64 timestamp()
	{
#ifdef KBE_USE_RDTSC
		return timestamp_rdtsc();
#else // KBE_USE_RDTSC
		if (g_timingMethod == RDTSC_TIMING_METHOD)
			return timestamp_rdtsc();
		else if (g_timingMethod == GET_TIME_OF_DAY_TIMING_METHOD)
			return timestamp_gettimeofday();
		else // GET_TIME_TIMING_METHOD
			return timestamp_gettime();

#endif // KBE_USE_RDTSC
	}

#elif defined(_WIN32)

#ifdef KBE_USE_RDTSC
#pragma warning (push)
#pragma warning (disable: 4035)
	inline uint64 timestamp()
	{
		__asm rdtsc
	}
#pragma warning (pop)
#else // KBE_USE_RDTSC

#include <windows.h>

	inline uint64 timestamp()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}

#endif // KBE_USE_RDTSC

#else
#error Unsupported platform!
#endif

	uint64 stampsPerSecond();
	double stampsPerSecondD();

	uint64 stampsPerSecond_rdtsc();
	double stampsPerSecondD_rdtsc();

	uint64 stampsPerSecond_gettimeofday();
	double stampsPerSecondD_gettimeofday();

	inline double stampsToSeconds(uint64 stamps)
	{
		return double(stamps) / stampsPerSecondD();
	}

	// -----------------------------------------------------------------------------
	class TimeStamp
	{
	public:
		TimeStamp(uint64 stamps = 0) :
			stamp_(stamps)
		{
		}

		operator uint64& ()
		{
			return stamp_;
		}

		operator uint64() const
		{
			return stamp_;
		}

		inline uint64 stamp() { return stamp_; }

		inline double inSeconds() const;
		inline void setInSeconds(double seconds);

		inline static double toSeconds(uint64 stamps);
		inline static TimeStamp fromSeconds(double seconds);

		uint64	stamp_;
	};


	inline double TimeStamp::toSeconds(uint64 stamps)
	{
		return double(stamps) / stampsPerSecondD();
	}

	inline TimeStamp TimeStamp::fromSeconds(double seconds)
	{
		return uint64(seconds * stampsPerSecondD());
	}

	inline double TimeStamp::inSeconds() const
	{
		return toSeconds(stamp_);
	}

	inline void TimeStamp::setInSeconds(double seconds)
	{
		stamp_ = fromSeconds(seconds);
	}



}

#endif // KBE_TIMESTAMP_H
