// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBEVERSION_H
#define KBEVERSION_H

#include "common/platform.h"

namespace KBEngine{
	
#define KBE_VERSION_MAJOR 2
#define KBE_VERSION_MINOR 4
#define KBE_VERSION_PATCH 4


namespace KBEVersion
{
	const std::string & versionString();
	void setScriptVersion(const std::string& ver);
	const std::string & scriptVersionString();
}

}
#endif // KBEVERSION_H
