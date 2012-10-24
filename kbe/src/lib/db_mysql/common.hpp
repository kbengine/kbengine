/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#ifndef __DB_SQL_COMMON_H__
#define __DB_SQL_COMMON_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine{ 

struct SQL_OP_TABLE_VAL_STRUCT
{
	char sqlval[MAX_BUF];
	char sqlkey[MAX_BUF];
	std::string extraDatas;
	std::string parentTableName;
	DBID parentTableID;
};

typedef std::vector< std::tr1::shared_ptr<SQL_OP_TABLE_VAL_STRUCT>  > SQL_OP_TABLE_VAL;
typedef std::tr1::unordered_map< std::string/*tableName*/, SQL_OP_TABLE_VAL > SQL_OP_TABLE;

}
#endif
