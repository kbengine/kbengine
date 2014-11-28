/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef KBE_VOLATILEINFO_HPP
#define KBE_VOLATILEINFO_HPP

#include "common/common.hpp"
#include "helper/debug_helper.hpp"
#include "pyscript/scriptobject.hpp"	

namespace KBEngine{

class VolatileInfo
{
public:
	static const float ALWAYS;

	VolatileInfo(float position = VolatileInfo::ALWAYS, float yaw = VolatileInfo::ALWAYS, 
		float roll = VolatileInfo::ALWAYS, float pitch = VolatileInfo::ALWAYS):

	position_(position),
	yaw_(yaw),
	roll_(roll),
	pitch_(pitch)
	{
	}

	virtual ~VolatileInfo(){
	}
	
	float position()const{ return position_; };
	float yaw()const{ return yaw_; };
	float roll()const{ return roll_; };
	float pitch()const{ return pitch_; };

	void position(float v){ 
		position_ = v; 
	};

	void yaw(float v){ 
		yaw_ = v;
	};

	void roll(float v){ 
		roll_ = v;
	};

	void pitch(float v){ 
		pitch_ = v;
	};

protected:	
	float position_;
	float yaw_;
	float roll_;
	float pitch_;
};

}


#endif // KBE_VOLATILEINFO_HPP
