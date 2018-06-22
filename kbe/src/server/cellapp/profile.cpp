// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "profile.h"	
namespace KBEngine{	

ProfileVal SCRIPTCALL_PROFILE("scriptCall");
ProfileVal ONMOVE_PROFILE("onMove");
ProfileVal ON_NAVIGATE_PROFILE("onNavigate");
ProfileVal CLIENT_UPDATE_PROFILE("clientUpdate");
ProfileVal ONTIMER_PROFILE("onTimer");

EventHistoryStats g_privateClientEventHistoryStats("PrivateClientEvents");
EventHistoryStats g_publicClientEventHistoryStats("PublicClientEvents");
EventHistoryStats g_publicCellEventHistoryStats("PublicCellEvents");

}
