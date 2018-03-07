/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_CONTROLLERS_H
#define KBE_CONTROLLERS_H

#include "controller.h"	

namespace KBEngine{

class Controllers
{
public:
	Controllers(ENTITY_ID entityID);
	~Controllers();

	bool add(KBEShared_ptr<Controller> pController);
	bool remove(KBEShared_ptr<Controller> pController);
	bool remove(uint32 id);
	
	void clear();

	uint32 freeID() { return ++lastid_; }

	typedef std::map<uint32, KBEShared_ptr< Controller > > CONTROLLERS_MAP;
	CONTROLLERS_MAP& objects() { return objects_; }

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

private:
	CONTROLLERS_MAP objects_;

	uint32 lastid_;

	ENTITY_ID entityID_;
};

}
#endif // KBE_CONTROLLERS_H
