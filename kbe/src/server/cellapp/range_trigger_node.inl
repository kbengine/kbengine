// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine{

//-------------------------------------------------------------------------------------
INLINE bool RangeTriggerNode::isInXRange(CoordinateNode * pNode)
{
	float originX = pRangeTrigger_->origin()->xx();

	volatile float lowerBound = originX - fabs(range_xz_);
	volatile float upperBound = originX + fabs(range_xz_);
	return (pNode->xx() >= lowerBound) && (pNode->xx() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTriggerNode::isInYRange(CoordinateNode * pNode)
{
	float originY = pRangeTrigger_->origin()->yy();

	volatile float lowerBound = originY - fabs(range_y_);
	volatile float upperBound = originY + fabs(range_y_);
	return (pNode->yy() >= lowerBound) && (pNode->yy() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTriggerNode::isInZRange(CoordinateNode * pNode)
{
	float originZ = pRangeTrigger_->origin()->zz();

	volatile float lowerBound = originZ - fabs(range_xz_);
	volatile float upperBound = originZ + fabs(range_xz_);
	return (pNode->zz() >= lowerBound) && (pNode->zz() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTriggerNode::wasInXRange(CoordinateNode * pNode)
{
	float originX = old_xx() - old_range_xz_;

	volatile float lowerBound = originX - fabs(old_range_xz_);
	volatile float upperBound = originX + fabs(old_range_xz_);
	return (pNode->old_xx() >= lowerBound) && (pNode->old_xx() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTriggerNode::wasInZRange(CoordinateNode * pNode)
{
	float originZ = old_zz() - old_range_xz_;

	volatile float lowerBound = originZ - fabs(old_range_xz_);
	volatile float upperBound = originZ + fabs(old_range_xz_);
	return (pNode->old_zz() >= lowerBound) && (pNode->old_zz() <= upperBound);
}

//-------------------------------------------------------------------------------------
INLINE void RangeTriggerNode::range(float xz, float y)
{
	range_xz_ = xz;
	range_y_ = y;
}

//-------------------------------------------------------------------------------------
INLINE void RangeTriggerNode::old_range(float xz, float y)
{
	old_range_xz_ = xz;
	old_range_y_ = y;
}

//-------------------------------------------------------------------------------------
INLINE float RangeTriggerNode::range_xz() const
{
	return range_xz_;
}

//-------------------------------------------------------------------------------------
INLINE float RangeTriggerNode::range_y() const
{
	return range_y_;
}

//-------------------------------------------------------------------------------------
INLINE RangeTrigger* RangeTriggerNode::pRangeTrigger() const
{
	return pRangeTrigger_;
}

//-------------------------------------------------------------------------------------
INLINE void RangeTriggerNode::pRangeTrigger(RangeTrigger* pRangeTrigger)
{
	pRangeTrigger_ = pRangeTrigger;
}

//-------------------------------------------------------------------------------------
INLINE bool RangeTriggerNode::isPositive() const
{
	return hasFlags(COORDINATE_NODE_FLAG_POSITIVE_BOUNDARY);
}

//-------------------------------------------------------------------------------------
}
