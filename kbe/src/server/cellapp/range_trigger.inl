// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine{

//-------------------------------------------------------------------------------------
INLINE void RangeTrigger::range(float xz, float y)
{
	range_xz_ = fabs(xz);
	range_y_ = fabs(y);
}

//-------------------------------------------------------------------------------------
INLINE float RangeTrigger::range_xz() const
{
	return range_xz_;
}

//-------------------------------------------------------------------------------------
INLINE float RangeTrigger::range_y() const
{
	return range_y_;
}

//-------------------------------------------------------------------------------------
INLINE CoordinateNode* RangeTrigger::origin() const
{
	return origin_;
}

//-------------------------------------------------------------------------------------
INLINE void RangeTrigger::origin(CoordinateNode* pCoordinateNode)
{
	origin_ = pCoordinateNode;
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTrigger::isInstalled() const
{
	return positiveBoundary_ && negativeBoundary_;
}

//-------------------------------------------------------------------------------------
}
