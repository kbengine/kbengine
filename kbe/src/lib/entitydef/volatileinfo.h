// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_VOLATILEINFO_H
#define KBE_VOLATILEINFO_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "pyscript/scriptobject.h"	

namespace KBEngine{

class VolatileInfo : public script::ScriptObject
{
	/**
	子类化 将一些py操作填充进派生类
	*/
	INSTANCE_SCRIPT_HREADER(VolatileInfo, ScriptObject)

public:
	static const float ALWAYS;
	static const float NEVER;

	VolatileInfo(float position = VolatileInfo::ALWAYS, float yaw = VolatileInfo::ALWAYS, 
		float roll = VolatileInfo::ALWAYS, float pitch = VolatileInfo::ALWAYS):
		ScriptObject(getScriptType(), false),
		position_(position),
		yaw_(yaw),
		roll_(roll),
		pitch_(pitch),
		optimized_(true)
	{
	}

	VolatileInfo(const VolatileInfo& info) :
		ScriptObject(getScriptType(), false),
		position_(info.position_),
		yaw_(info.yaw_),
		roll_(info.roll_),
		pitch_(info.pitch_),
		optimized_(true)
	{
	}

	virtual ~VolatileInfo(){
	}

	void update(bool pos_isALWAYS, bool yaw_isALWAYS, bool pitch_isALWAYS, bool roll_isALWAYS)
	{
		position_ = pos_isALWAYS ? ALWAYS : NEVER;
		yaw_ = yaw_isALWAYS ? ALWAYS : NEVER;
		roll_ = pitch_isALWAYS ? ALWAYS : NEVER;
		pitch_ = roll_isALWAYS ? ALWAYS : NEVER;
	}

	void update(float pos, float yaw, float pitch, float roll)
	{
		position_ = pos;
		yaw_ = yaw;
		roll_ = pitch;
		pitch_ = roll;
	}

	void updateToNEVER()
	{
		position_ = NEVER;
		yaw_ = NEVER;
		roll_ = NEVER;
		pitch_ = NEVER;
	}

	void updateToALWAYS()
	{
		position_ = ALWAYS;
		yaw_ = ALWAYS;
		roll_ = ALWAYS;
		pitch_ = ALWAYS;
	}

	float position() const { return position_; };
	float yaw() const { return yaw_; };
	float roll() const { return roll_; };
	float pitch() const { return pitch_; };

	void position(float v) { 
		position_ = v; 
	};

	void yaw(float v) { 
		yaw_ = v;
	};

	void roll(float v) { 
		roll_ = v;
	};

	void pitch(float v) { 
		pitch_ = v;
	};

	bool optimized() const {
		return optimized_;
	}

	void optimized(bool v) {
		optimized_ = v;
	};

	DECLARE_PY_GETSET_MOTHOD(pyGetPosition, pySetPosition);
	DECLARE_PY_GETSET_MOTHOD(pyGetYaw, pySetYaw);
	DECLARE_PY_GETSET_MOTHOD(pyGetPitch, pySetPitch);
	DECLARE_PY_GETSET_MOTHOD(pyGetRoll, pySetRoll);

	DECLARE_PY_GETSET_MOTHOD(pyGetOptimized, pySetOptimized);

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

protected:	
	float position_;
	float yaw_;
	float roll_;
	float pitch_;

	bool optimized_;
};

}


#endif // KBE_VOLATILEINFO_H
