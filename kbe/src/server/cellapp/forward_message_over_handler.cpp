#include "cellapp.hpp"
#include "forward_message_over_handler.hpp"
#include "entitydef/entities.hpp"
#include "entity.hpp"
#include "space.hpp"
#include "spaces.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
FMH_Baseapp_onEntityGetCell::FMH_Baseapp_onEntityGetCell(Entity* e, SPACE_ID spaceID):
_e(e),
_spaceID(spaceID)
{
}

//-------------------------------------------------------------------------------------
void FMH_Baseapp_onEntityGetCell::process()
{
	KBE_ASSERT(_e != NULL);
	
	Space* space = Spaces::findSpace(_spaceID);
	
	if(space == NULL)
	{
		ERROR_MSG("FMH_Baseapp_onEntityGetCell::process: not found space(%d), %s %d.\n", 
			_spaceID, _e->getScriptName(), _e->getID());
		return;
	}

	// Ìí¼Óµ½space
	space->addEntity(_e);
	_e->initializeScript();
}

//-------------------------------------------------------------------------------------
}
