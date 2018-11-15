// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "profile.h"	
namespace KBEngine{	

ProfileVal SCRIPTCALL_PROFILE("scriptCall");
ProfileVal SCRIPTCALL_CREATEACCOUNT_PROFILE("onRequestCreateAccount");
ProfileVal SCRIPTCALL_ACCOUNTLOGIN_PROFILE("onRequestAccountLogin");
ProfileVal SCRIPTCALL_CHARGE_PROFILE("onRequestCharge");

EventHistoryStats g_privateClientEventHistoryStats("PrivateClientEvents");
EventHistoryStats g_publicClientEventHistoryStats("PublicClientEvents");

}
