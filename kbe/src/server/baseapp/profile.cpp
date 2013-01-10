#include "profile.hpp"	
namespace KBEngine{	

ProfileVal SCRIPTCALL_PROFILE("scriptCall");
ProfileVal ONTIMER_PROFILE("onTimer");

EventHistoryStats g_privateClientEventHistoryStats("PrivateClientEvents");
EventHistoryStats g_publicClientEventHistoryStats("PublicClientEvents");

}
