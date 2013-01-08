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
INLINE Entity* Witness::pEntity()
{
	return pEntity_;
}

//-------------------------------------------------------------------------------------
INLINE void Witness::pEntity(Entity* pEntity)
{
	pEntity_ = pEntity;
}

//-------------------------------------------------------------------------------------
INLINE void Witness::setAoiRadius(float radius, float hyst)
{
	aoiRadius_ = radius;
	aoiHysteresisArea_ = hyst;

	if(aoiRadius_ + aoiHysteresisArea_ > CELL_BORDER_WIDTH)
	{
		aoiRadius_ = CELL_BORDER_WIDTH - 15.0f;
		aoiHysteresisArea_ = 15.0f;
	}
}

//-------------------------------------------------------------------------------------
}
