#include "MapUtil.h"
#include <math.h>
#include "basic_types.h"

NS_GC_BEGIN

#define M_PI_RECIPROCAL	0.31830988618379067153776752674503
#define M_PI_RECIPROCAL_DOT_180	57.3f

int MapUtil::judgeDir(float startX, float startY, float endX, float endY)
{
	int dir;
	float _local6 = (atan2((endY - startY), (endX - startX)))*M_PI_RECIPROCAL_DOT_180;
	if ((((_local6 > -157.5)) && ((_local6 < -112.5))))//左下
	{
		dir = DIRECTION_LEFT;
	}
	else
	{
		if ((((_local6 > -112.5)) && ((_local6 < -67.5))))//下
		{
			dir = DIRECTION_DOWN;
		}
		else
		{
			if ((((_local6 > -67.5)) && ((_local6 < -22.5))))//右下
			{
				dir = DIRECTION_RIGHT;
			}
			else
			{
				if ((((_local6 > -22.5)) && ((_local6 < 22.5))))//右
				{
					dir = DIRECTION_RIGHT;
				}
				else
				{
					if ((((_local6 > 22.5)) && ((_local6 < 67.5))))//右上
					{
						dir = DIRECTION_RIGHT;
					}
					else
					{
						if ((((_local6 > 67.5)) && ((_local6 < 112.5))))//上
						{
							dir = DIRECTION_UP;
						}
						else
						{
							if ((((_local6 > 112.5)) && ((_local6 < 157.5))))//左上
							{
								dir = DIRECTION_LEFT;
							}
							else//左
							{
								dir = DIRECTION_LEFT;
							};
						};
					};
				};
			};
		};
	};
	return (dir);
}
int MapUtil::getPathDistance(const tPoint& start, const tPoint& end)
{
	int dx = start.x-end.x;
	int dy = start.y-end.y;
	dx = dx>0?dx:-dx;
	dy = dy>0?dy:-dy;
	return (sqrt(0.8*(dx-dy)*(dx-dy) + 0.2*(dx+dy)*(dx+dy)));
}

NS_GC_END