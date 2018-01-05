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

#ifndef KBE_PYPROFILE_HANDLER_H
#define KBE_PYPROFILE_HANDLER_H

#include "helper/profile_handler.h"

namespace KBEngine { 

class PyProfileHandler : public ProfileHandler
{
public:
	PyProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr);
	virtual ~PyProfileHandler();
	
	void timeout();
	void sendStream(MemoryStream* s);
};

class PyTickProfileHandler : public Task,
                             public ProfileHandler
{
public:
	PyTickProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen,
		std::string name, const Network::Address& addr);
	virtual ~PyTickProfileHandler();

	virtual void timeout();
	virtual bool process();
	virtual void sendStream(MemoryStream* s);
};


}

#endif // KBE_PYPROFILE_HANDLER_H
