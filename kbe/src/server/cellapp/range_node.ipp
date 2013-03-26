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


namespace KBEngine{

//-------------------------------------------------------------------------------------
INLINE RangeNode* RangeNode::pPrevX()const { return pPrevX_; }
INLINE RangeNode* RangeNode::pNextX()const { return pNextX_; }
INLINE RangeNode* RangeNode::pPrevZ()const { return pPrevZ_; }
INLINE RangeNode* RangeNode::pNextZ()const { return pNextZ_; }

//-------------------------------------------------------------------------------------
INLINE void RangeNode::pPrevX(RangeNode* pNode){ pPrevX_ = pNode; }
INLINE void RangeNode::pNextX(RangeNode* pNode){ pNextX_ = pNode; }
INLINE void RangeNode::pPrevZ(RangeNode* pNode){ pPrevZ_ = pNode; }
INLINE void RangeNode::pNextZ(RangeNode* pNode){ pNextZ_ = pNode; }
	
//-------------------------------------------------------------------------------------
}
