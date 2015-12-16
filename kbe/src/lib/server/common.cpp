/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "common.h"
#include "server/serverconfig.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
int32 secondsToTicks(float seconds, int lowerBound)
{
	return std::max(lowerBound, int(floorf(seconds * g_kbeSrvConfig.gameUpdateHertz() + 0.5f)));
}

//-------------------------------------------------------------------------------------
uint16 datatype2id(std::string datatype)
{
	std::transform(datatype.begin(), datatype.end(), datatype.begin(), toupper);	
	if(datatype == "STRING" || datatype == "STD::STRING")
		return 1;
	else if(datatype == "UINT8" || datatype == "BOOL" || datatype == "DATATYPE" || datatype == "CHAR" || datatype == "DETAIL_TYPE" ||
		datatype == "MAIL_TYPE")
		return 2;
	else if(datatype == "UINT16" || datatype == "UNSIGNED SHORT" || datatype == "SERVER_ERROR_CODE" || datatype == "ENTITY_TYPE" ||
		datatype == "ENTITY_PROPERTY_UID" || datatype == "ENTITY_METHOD_UID" || datatype == "ENTITY_SCRIPT_UID" || datatype == "DATATYPE_UID")
		return 3;
	else if(datatype == "UINT32" || datatype == "UINT" || datatype == "UNSIGNED INT" ||datatype == "ARRAYSIZE" || datatype == "SPACE_ID" || datatype == "GAME_TIME" ||
		datatype == "TIMER_ID")
		return 4;
	else if(datatype == "UINT64" || datatype == "DBID" || datatype == "COMPONENT_ID")
		return 5;
	else if(datatype == "INT8" || datatype == "COMPONENT_ORDER")
		return 6;
	else if(datatype == "INT16" || datatype == "SHORT")
		return 7;
	else if(datatype == "INT32" || datatype == "INT" ||datatype == "ENTITY_ID" || datatype == "CALLBACK_ID" || datatype == "COMPONENT_TYPE")
		return 8;
	else if(datatype == "INT64")
		return 9;
	else if(datatype == "PYTHON" || datatype == "PY_DICT" || datatype == "PY_TUPLE" || datatype == "PY_LIST" || datatype == "MAILBOX")
		return 10;
	else if(datatype == "BLOB")
		return 11;
	else if(datatype == "UNICODE")
		return 12;
	else if(datatype == "FLOAT")
		return 13;
	else if(datatype == "DOUBLE")
		return 14;
	else if(datatype == "VECTOR2")
		return 15;
	else if(datatype == "VECTOR3")
		return 16;
	else if(datatype == "VECTOR4")
		return 17;
	else if(datatype == "FIXED_DICT")
		return 18;
	else if(datatype == "ARRAY")
		return 19;
	else
		assert(false);

	return 0;
}

//-------------------------------------------------------------------------------------
}
