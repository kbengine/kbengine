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

#ifndef __BASEAPPMGR_BASEAPP_H__
#define __BASEAPPMGR_BASEAPP_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine{ 

class Baseapp
{
public:
	Baseapp();
	virtual ~Baseapp();
	
	ENTITY_ID numBases()const { return numBases_; }
	void numBases(ENTITY_ID num) { numBases_ = num; }
	
	ENTITY_ID numProxices()const { return numProxices_; }
	void numProxices(ENTITY_ID num) { numProxices_ = num; }
	
	float load()const { return load_; }
	void load(int32 v) { load_ = v; }

protected:
	ENTITY_ID numBases_;
	ENTITY_ID numProxices_;
	float load_;
};

}
#endif
