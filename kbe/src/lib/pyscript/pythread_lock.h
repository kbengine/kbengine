// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
