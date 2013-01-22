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
INLINE void Entity::setDirection(Direction3D& dir)
{ 
	direction_ = dir; 
}

//-------------------------------------------------------------------------------------
INLINE Direction3D& Entity::getDirection()
{ 
	return direction_; 
}

//-------------------------------------------------------------------------------------
INLINE Position3D& Entity::getPosition()
{
	return position_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::pClientApp(ClientObjectBase* p)
{ 
	pClientApp_ = p; 
}

//-------------------------------------------------------------------------------------
INLINE ClientObjectBase* Entity::pClientApp()const
{
	return pClientApp_;
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Entity::getBaseMailbox()const
{ 
	return baseMailbox_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setCellMailbox(EntityMailbox* mailbox)
{ 
	cellMailbox_ = mailbox; 
}

//-------------------------------------------------------------------------------------
INLINE EntityMailbox* Entity::getCellMailbox()const
{ 
	return cellMailbox_; 
}

//-------------------------------------------------------------------------------------
INLINE void Entity::setBaseMailbox(EntityMailbox* mailbox)
{ 
	baseMailbox_ = mailbox; 
}

//-------------------------------------------------------------------------------------
}

