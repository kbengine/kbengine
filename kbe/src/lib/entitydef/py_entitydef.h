// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_ENTITYDEF_H
#define KBE_SCRIPT_ENTITYDEF_H

#include "common.h"

namespace KBEngine{ namespace script{ namespace entitydef {
	
/** °²×°entitydefÄ£¿é */
bool installModule(const char* moduleName);
bool uninstallModule();

bool initialize(std::vector<PyTypeObject*>& scriptBaseTypes,
	COMPONENT_TYPE loadComponentType);

bool process();

bool finalise(bool isReload = false);

void reload(bool fullReload);

bool initializeWatcher();

}
}
}

#endif // KBE_SCRIPT_ENTITYDEF_H
