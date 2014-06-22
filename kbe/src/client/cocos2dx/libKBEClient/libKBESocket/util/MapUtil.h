#pragma once

#include "KBEClientCoreMacros.h"

NS_GC_BEGIN

#define DIRECTION_RIGHT_DOWN 3
#define DIRECTION_LEFT_DOWN 1
#define DIRECTION_LEFT_UP 7
#define DIRECTION_RIGHT_UP 9
#define DIRECTION_DOWN 2
#define DIRECTION_LEFT 4
#define DIRECTION_UP 8
#define DIRECTION_RIGHT 6

class tPoint;
class MapUtil
{
public:
	static int judgeDir(float startX, float startY, float endX, float endY);//获取两个点之间的方向
	static int getPathDistance(const tPoint& start, const tPoint& end);//求两个tile之间的距离（棱形）
};

NS_GC_END