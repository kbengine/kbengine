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

#ifndef KBE_SPACEMANAGER_HPP
#define KBE_SPACEMANAGER_HPP

// common include
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "space.hpp"
// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32	
#else
// linux include
#endif

namespace KBEngine{

class Spaces
{
public:
	Spaces();
	~Spaces();
	
	static void finalise();

	typedef std::map<SPACE_ID, KBEShared_ptr<Space> > SPACES;

	/** 
		����һ���µ�space 
	*/
	static Space* createNewSpace(SPACE_ID spaceID);
	
	/**
		����һ��space
	*/
	static bool destroySpace(SPACE_ID spaceID, ENTITY_ID entityID);

	/** 
		Ѱ��һ��ָ��space 
	*/
	static Space* findSpace(SPACE_ID spaceID);
	
	/** 
		�������е�space 
	*/
	static void update();
protected:
	static SPACES spaces_;
};

}
#endif
