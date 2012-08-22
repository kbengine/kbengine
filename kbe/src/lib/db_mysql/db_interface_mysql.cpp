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
	
	kbe_snprintf(db_name_, MAX_BUF, "%s", databaseName);
    pMysql_ = mysql_init(0);

	if(pMysql_ == NULL)
	{
		return false;
	}

    pMysql_ = mysql_real_connect(mysql(), db_ip_, db_username_, 
    	db_password_, db_name_, db_port_, 0, CLIENT_MULTI_STATEMENTS);  
    
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
bool DBInterfaceMysql::query(const char* strCommand, uint32 size)
{
	if(pMysql_ == NULL)
	{
		ERROR_MSG("DBInterfaceMysql::query: has no attach(db).\n");
		return false;
	}

    int nResult = mysql_real_query(pMysql_, strCommand, size);  
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
bool DBInterfaceMysql::execute(const char* strCommand, uint32 size, MemoryStream * resdata)
{
	bool result = this->query(strCommand, size);

	if (!result)
	{
		return false;
	}

	MYSQL_RES * pResult = mysql_store_result(mysql());

	if(pResult)
	{
		if (resdata != NULL)
		{
			uint32 nrows = (uint32)mysql_num_rows(pResult);
			uint32 nfields = (uint32)mysql_num_fields(pResult);

			(*resdata) << nfields << nrows;

			MYSQL_ROW arow;

			while((arow = mysql_fetch_row(pResult)) != NULL)
			{
				unsigned long *lengths = mysql_fetch_lengths(pResult);

				for (uint32 i = 0; i < nfields; i++)
				{
					if (arow[i] == NULL)
					{
						std::string null = "NULL";
						resdata->appendBlob(null.c_str(), null.size());
					}
					else
					{
						resdata->appendBlob(arow[i], lengths[i]);
					}
				}
			}
		}

		mysql_free_result(pResult);
	}
	else
	{
		uint32 nfields = 0;
		uint64 affectedRows = mysql()->affected_rows;
		(*resdata) << ""; // errormsg
		(*resdata) << nfields;
		(*resdata) << affectedRows;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::getTableNames( std::vector<std::string>& tableNames, const char * pattern)
{
	if(pMysql_ == NULL)
	{
		ERROR_MSG("DBInterfaceMysql::query: has no attach(db).\n");
		return false;
	}

	tableNames.clear();

	MYSQL_RES * pResult = mysql_list_tables(pMysql_, pattern);

	if(pResult)
	{
		tableNames.reserve((unsigned int)mysql_num_rows(pResult));

		MYSQL_ROW row;
		while((row = mysql_fetch_row(pResult)) != NULL)
		{
			unsigned long *lengths = mysql_fetch_lengths(pResult);
			tableNames.push_back(std::string(row[0], lengths[0]));
		}

		mysql_free_result(pResult);
	}

	return true;
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceMysql::c_str()
{
	static char strdescr[MAX_BUF];
	kbe_snprintf(strdescr, MAX_BUF, "dbtype=mysql, ip=%s, port=%u, currdatabase=%s, username=%s, connected=%s.\n", 
		db_ip_, db_port_, db_name_, db_username_, pMysql_ == NULL ? "no" : "yes");

	return strdescr;
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceMysql::getstrerror()
{
	return mysql_error(pMysql_);
}


//-------------------------------------------------------------------------------------
int DBInterfaceMysql::getlasterror()
{
	return mysql_errno(pMysql_);
}
//-------------------------------------------------------------------------------------
}
