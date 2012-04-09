#include "kbeversion.hpp"
#include "helper/debug_helper.hpp"
namespace KBEngine{
namespace KBEVersion
{
std::string g_versionString;


const std::string & versionString()
{
	if(g_versionString.size() == 0)
	{
		char buf[ 256 ];
		sprintf( buf, "%d.%d.%d",
				KBE_VERSION_MAJOR, KBE_VERSION_MINOR, KBE_VERSION_PATCH );
		g_versionString = buf;
	}
	return g_versionString;
}




}
} 


