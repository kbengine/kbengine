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


namespace KBEngine{

	
//-------------------------------------------------------------------------------------
INLINE CoordinateNode * CoordinateSystem::pFirstXNode() const { return first_x_coordinateNode_; }

//-------------------------------------------------------------------------------------
INLINE CoordinateNode * CoordinateSystem::pFirstYNode() const { return first_y_coordinateNode_; }

//-------------------------------------------------------------------------------------
INLINE CoordinateNode * CoordinateSystem::pFirstZNode() const { return first_z_coordinateNode_; }

//-------------------------------------------------------------------------------------
INLINE uint32 CoordinateSystem::size() const{ return size_; }

//-------------------------------------------------------------------------------------
INLINE bool CoordinateSystem::isEmpty() const 
{ 
	return first_x_coordinateNode_ == NULL && first_y_coordinateNode_ == NULL && first_z_coordinateNode_ == NULL;
}

//-------------------------------------------------------------------------------------
INLINE void CoordinateSystem::incUpdating()
{
	++updating_;
}

//-------------------------------------------------------------------------------------
INLINE void CoordinateSystem::decUpdating()
{
	--updating_;
}

//-------------------------------------------------------------------------------------

}
