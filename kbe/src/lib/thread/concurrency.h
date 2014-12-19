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



#ifndef KBE_CONCURENCY_HPP
#define KBE_CONCURENCY_HPP

#include "common/platform.h"
#include "helper/debug_helper.h"
namespace KBEngine{

extern void (*pMainThreadIdleStartFunc)();
extern void (*pMainThreadIdleEndFunc)();

namespace KBEConcurrency
{
inline void startMainThreadIdling()
{
	if(pMainThreadIdleStartFunc)
		(*pMainThreadIdleStartFunc)();
}

inline void endMainThreadIdling()
{
	if(pMainThreadIdleEndFunc)
		(*pMainThreadIdleEndFunc)();
}

inline
void setMainThreadIdleFunctions( void (*pStartFunc)(), void (*pEndFunc)() )
{
	pMainThreadIdleStartFunc = pStartFunc;
	pMainThreadIdleEndFunc = pEndFunc;
}

}

}

#endif // KBE_CONCURENCY_HPP
