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

#ifndef KBENGINE_SCRIPT_LOCK_H
#define KBENGINE_SCRIPT_LOCK_H

#include "Python.h"

namespace KBEngine{ namespace script{


class PyThreadStateLock
{
public:
    PyThreadStateLock(void)
    {
#ifndef KBE_SINGLE_THREADED
        state = PyGILState_Ensure();
#endif
    }

    ~PyThreadStateLock(void)
    {
#ifndef KBE_SINGLE_THREADED
         PyGILState_Release( state );
 #endif
    }
private:
#ifndef KBE_SINGLE_THREADED
    PyGILState_STATE state;
#endif
};

}
}

#endif // KBENGINE_SCRIPT_LOCK_H
