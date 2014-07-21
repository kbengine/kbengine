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

#include "cellapp.hpp"
#include "ghost_manager.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
GhostManager::GhostManager(Mercury::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
entityIDs_(),
ghost_route_()

{
	networkInterface.mainDispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
GhostManager::~GhostManager()
{
	networkInterface_.mainDispatcher().cancelFrequentTask(this);
	DEBUG_MSG("GhostManager::~GhostManager()\n");
}

//-------------------------------------------------------------------------------------
bool GhostManager::process()
{
	return true;
}

//-------------------------------------------------------------------------------------

}
