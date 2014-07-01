#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include "KBEClientCoreMacros.h"
#include <math.h>

NS_GC_BEGIN

#ifdef _MSC_VER

typedef	signed		__int8		int8;
typedef	unsigned	__int8		uint8;
typedef	signed		__int16		int16;
typedef	unsigned	__int16		uint16;
typedef	signed		__int32		int32;
typedef	unsigned	__int32		uint32;
typedef	signed		__int64		int64;
typedef	unsigned	__int64		uint64;

#else
#include <stdint.h>
typedef	int8_t		int8;
typedef	uint8_t		uint8;
typedef	int16_t		int16;
typedef	uint16_t	uint16;
typedef	int32_t		int32;
typedef	uint32_t	uint32;
typedef	int64_t		int64;
typedef	uint64_t	uint64;

#endif

class tPoint
{
public:
	int x;
	int y;
public:
	tPoint()
	{
		x = 0;
		y = 0;
	}
	tPoint(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
	bool operator== (const tPoint& Right)
	{
		if (Right.x==x && Right.y==y)
		{
			return true;
		}
		return false;
	}
	bool operator< (const tPoint& Right)
	{
		return (x!=Right.x)?(x<Right.x):(y<Right.y);
	}
	float distance(const tPoint& target)
	{
		return sqrtf((float)((x-target.x)*(x-target.x)+(y-target.y)*(y-target.y)));
	}
};
inline bool operator< (const tPoint& Left, const tPoint& Right)
{
	return (Left.x!=Right.x)?(Left.x<Right.x):(Left.y<Right.y);
}

NS_GC_END

#endif
