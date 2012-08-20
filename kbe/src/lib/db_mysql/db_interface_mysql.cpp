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


#include "db_interface_mysql.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
DBInterfaceMysql::DBInterfaceMysql() :
DBInterface(),
pMysql_(NULL)
{
}

//-------------------------------------------------------------------------------------
DBInterfaceMysql::~DBInterfaceMysql()
{
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::attach(const char* databaseName)
{
	if(db_port_ == 0)
		db_port_ = 3306;

    pMysql_ = mysql_init(0);
    pMysql_ = mysql_real_connect(mysql(), db_ip_, db_username_, 
    	db_password_, db_name_, db_port_, 0, 0);  
    
	if(mysql())
		mysql_select_db(mysql(), databaseName); 
    return mysql() != NULL;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::detach()
{
	if(mysql())
	{
		::mysql_close(mysql());
		pMysql_ = NULL;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::query(const char* strCommand)
{
	if(pMysql_ == NULL)
	{
		ERROR_MSG("DBInterfaceMysql::query: has no attach(db).\n");
		return false;
	}

    int nResult = mysql_query(pMysql_, strCommand);  
    if(nResult != 0)  
    {  
		ERROR_MSG("DBInterfaceMysql::query: mysql is error(%d:%s)!\n", 
			mysql_errno(pMysql_), mysql_error(pMysql_)); 
        return false;
    }  
    else
    {
		INFO_MSG("DBInterfaceMysql::query: successfully!\n"); 
    }
    
    return true;
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceMysql::c_str()
{
	static char strdescr[MAX_BUF];
	kbe_snprintf(strdescr, MAX_BUF, "ip=%s, port=%u, currdatabase=%s, username=%s, connected=%s.\n", 
		db_ip_, db_port_, db_name_, db_username_, pMysql_ == NULL ? "no" : "yes");

	return strdescr;
}

//-------------------------------------------------------------------------------------
}
