/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#ifndef KBE_CELLAPPMGR_CELLAPP_H
#define KBE_CELLAPPMGR_CELLAPP_H

#include "spaces.h"
#include "common/common.h"
#include "helper/debug_helper.h"
#include "helper/watcher.h"

namespace KBEngine{ 

class Cellapp
{
public:
	Cellapp();
	virtual ~Cellapp();
	
	float load() const { return load_; }
	void load(float v) { load_ = v; }
	
	void destroy(){ isDestroyed_ = true; }
	bool isDestroyed() const { return isDestroyed_; }

	float initProgress() const{ return initProgress_; }
	void initProgress(float v){ initProgress_ = v; }

	ENTITY_ID numEntities() const { return numEntities_; }
	void numEntities(ENTITY_ID num) { numEntities_ = num; }

	uint32 flags() const { return flags_; }
	void flags(uint32 v) { flags_ = v; }
	
protected:
	ENTITY_ID numEntities_;

	float load_;

	bool isDestroyed_;

	Watchers watchers_;
	
	Spaces spaces_;

	float initProgress_;

	uint32 flags_;
};

}

#endif // KBE_CELLAPPMGR_CELLAPP_H
