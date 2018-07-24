// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#ifndef KBE_NAVIGATION_H
#define KBE_NAVIGATION_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "common/smartpointer.h"
#include "common/singleton.h"
#include "math/math.h"
#include "navigation_handle.h"

namespace KBEngine
{
/*
	µº∫Ω¿‡
*/
class Navigation : public Singleton<Navigation>
{
public:
	Navigation();
	virtual ~Navigation();
	
	void finalise();

	NavigationHandlePtr loadNavigation(std::string resPath, const std::map< int, std::string >& params);

	bool hasNavigation(std::string resPath);

	bool removeNavigation(std::string resPath);

	NavigationHandlePtr findNavigation(std::string resPath);

private:
	KBEUnordered_map<std::string, NavigationHandlePtr> navhandles_;
	KBEngine::thread::ThreadMutex mutex_;
};

}
#endif
