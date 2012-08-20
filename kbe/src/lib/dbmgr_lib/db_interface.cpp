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


#include "db_interface.hpp"
#include "db_mysql/db_interface_mysql.hpp"

namespace KBEngine { 
KBE_SINGLETON_INIT(DBUtil);

DBUtil g_DBUtil;

//-------------------------------------------------------------------------------------
DBUtil::DBUtil()
{
}

//-------------------------------------------------------------------------------------
DBUtil::~DBUtil()
{
}

//-------------------------------------------------------------------------------------
DBInterface* DBUtil::create(const char* dbtype, const char* ip, uint32 port, const char* db_username, 
		const char* db_password, uint16 db_numConnections)
{
	DBInterface* dbinterface = NULL;

	if(stricmp(dbtype, "mysql") == 0)
	{
		dbinterface = new DBInterfaceMysql;
	}

	kbe_snprintf(dbinterface->db_type_, MAX_BUF, dbtype);
	dbinterface->db_port_ = port;	
	kbe_snprintf(dbinterface->db_ip_, MAX_BUF, ip);
	kbe_snprintf(dbinterface->db_username_, MAX_BUF, db_username);
	kbe_snprintf(dbinterface->db_password_, MAX_BUF, db_password);
	dbinterface->db_numConnections_ = db_numConnections;
	
	return dbinterface;
}
//-------------------------------------------------------------------------------------
}
