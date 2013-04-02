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
#include "entity_table_mysql.hpp"
#include "db_exception.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
DBInterfaceMysql::DBInterfaceMysql(std::string characterSet, std::string collation) :
DBInterface(),
pMysql_(NULL),
hasLostConnection_(false),
inTransaction_(false),
lock_(NULL, false),
characterSet_(characterSet),
collation_(collation)
{
	lock_.pdbi(this);
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
	
	if(databaseName != NULL)
		kbe_snprintf(db_name_, MAX_BUF, "%s", databaseName);

	hasLostConnection_ = false;

	try
	{
		pMysql_ = mysql_init(0);
		if(pMysql_ == NULL)
		{
			ERROR_MSG("DBInterfaceMysql::attach: mysql_init is error!\n");
			return false;
		}
		
		DEBUG_MSG(boost::format("DBInterfaceMysql::mysql_real_connect: %1%:%2%\n") % db_ip_ % db_port_);
		if(mysql_real_connect(mysql(), db_ip_, db_username_, 
    		db_password_, db_name_, db_port_, NULL, 0)) // CLIENT_MULTI_STATEMENTS  
		{
			if(mysql_select_db(mysql(), db_name_) != 0)
			{
				ERROR_MSG(boost::format("DBInterfaceMysql::attach: Could not set active db[%1%]\n") %
					db_name_);

				return false;
			}
		}
		else
		{
			ERROR_MSG(boost::format("DBInterfaceMysql::attach: mysql_errno=%1%, mysql_error=%2%\n") %
				mysql_errno(pMysql_) % mysql_error(pMysql_));
			return false;
		}

		if (mysql_set_character_set(mysql(), "utf8") != 0)
		{
			ERROR_MSG("DBInterfaceMysql::attach: Could not set client connection character set to UTF-8\n" );
		}

		char characterset_sql[MAX_BUF];
		kbe_snprintf(characterset_sql, MAX_BUF, "ALTER DATABASE CHARACTER SET %s COLLATE %s", 
			characterSet_.c_str(), collation_.c_str());

		query(&characterset_sql[0], strlen(characterset_sql), false);
	}
	catch (std::exception& e)
	{
		ERROR_MSG(boost::format("DBInterfaceMysql::attach: %1%\n") % e.what());
		hasLostConnection_ = true;
		return false;
	}

    return mysql() != NULL && ping();
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::reattach()
{
	detach();

	bool ret = false;

	try
	{
		ret = attach();
	}
	catch (...)
	{
		return false;
	}

	return ret;
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
EntityTable* DBInterfaceMysql::createEntityTable()
{
	return new EntityTableMysql();
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::dropEntityTableFromDB(const char* tablename)
{
	KBE_ASSERT(tablename != NULL);
  
	DEBUG_MSG(boost::format("DBInterfaceMysql::dropEntityTableFromDB: %1%.\n") % tablename);

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "Drop table if exists %s;", tablename);
	return query(sql_str, strlen(sql_str));
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::dropEntityTableItemFromDB(const char* tablename, const char* tableItemName)
{
	KBE_ASSERT(tablename != NULL && tableItemName != NULL);
  
	DEBUG_MSG(boost::format("DBInterfaceMysql::dropEntityTableItemFromDB: %1% %2%.\n") % 
		tablename % tableItemName);

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "alter table %s drop column %s;", tablename, tableItemName);
	return query(sql_str, strlen(sql_str));
}

//-------------------------------------------------------------------------------------
void DBInterfaceMysql::throwError()
{
	DBException e( this );

	if (e.isLostConnection())
	{
		this->hasLostConnection(true);
	}

	throw e;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::query(const char* strCommand, uint32 size, bool showexecinfo)
{
	if(pMysql_ == NULL)
	{
		if(showexecinfo)
		{
			ERROR_MSG("DBInterfaceMysql::query: has no attach(db).\n");
		}

		return false;
	}

    int nResult = mysql_real_query(pMysql_, strCommand, size);  
    if(nResult != 0)  
    {  
		if(showexecinfo)
		{
			ERROR_MSG(boost::format("DBInterfaceMysql::query: mysql is error(%1%:%2%)!\n") % 
				mysql_errno(pMysql_) % mysql_error(pMysql_)); 
		}

		this->throwError();
        return false;
    }  
    else
    {
		if(showexecinfo)
		{
			INFO_MSG("DBInterfaceMysql::query: successfully!\n"); 
		}
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

	if(resdata == NULL)
	{
		return true;
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
bool DBInterfaceMysql::getTableNames(std::vector<std::string>& tableNames, const char * pattern)
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
bool DBInterfaceMysql::getTableItemNames(const char* tablename, std::vector<std::string>& itemNames)
{
	MYSQL_RES*	result = mysql_list_fields(mysql(), tablename, NULL);
	if(result)
	{
		unsigned int numFields = mysql_num_fields(result);
		MYSQL_FIELD* fields = mysql_fetch_fields(result);

		for(unsigned int i = 0; i < numFields; i++)
		{
			itemNames.push_back(fields[i].name);
		}
	}
	else
		return false;

	mysql_free_result(result);
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
void DBInterfaceMysql::getFields(TABLE_FIELDS& outs, const char* tablename)
{
	std::string sqlname = ENTITY_TABLE_PERFIX"_";
	sqlname += tablename;

	MYSQL_RES*	result = mysql_list_fields(mysql(), sqlname.c_str(), NULL);
	if(result == NULL)
	{
		ERROR_MSG(boost::format("EntityTableMysql::loadFields:%1%\n") % getstrerror());
		return;
	}

	unsigned int numFields;
	MYSQL_FIELD* fields;

	numFields = mysql_num_fields(result);
	fields = mysql_fetch_fields(result);

	for(unsigned int i=0; i<numFields; i++)
	{
		TABLE_FIELD& info = outs[fields[i].name];
		info.name = fields[i].name;
		info.length = fields[i].length;
		info.maxlength = fields[i].max_length;
		info.flags = fields[i].flags;
		info.type = fields[i].type;
	}

	mysql_free_result(result);
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::lock()
{
	lock_.start();
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::unlock()
{
	lock_.commit();
	lock_.end();
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::processException(std::exception & e)
{
	DBException* dbe = static_cast<DBException*>(&e);
	bool retry = false;

	if (dbe->isLostConnection())
	{
		INFO_MSG(boost::format("DBInterfaceMysql::processException: "
				"Thread %p lost connection to database. Exception: %s. "
				"Attempting to reconnect.\n") %
			this %
			dbe->what() );

		int attempts = 1;

		while (!this->reattach())
		{
			ERROR_MSG(boost::format("DBInterfaceMysql::processException: "
							"Thread %p reconnect(%s) attempt %d failed(%s).\n") %
						this %
						db_name_ %
						attempts %
						getLastError());

			KBEngine::sleep(30);
			++attempts;
		}

		INFO_MSG(boost::format("DBInterfaceMysql::processException: "
					"Thread %p reconnected(%s). Attempts = %d\n") %
				this %
				db_name_ %
				attempts);

		retry = true;
	}
	else if (dbe->shouldRetry())
	{
		WARNING_MSG(boost::format("DBInterfaceMysql::processException: Retrying %1%\n") %
				this );

		retry = true;
	}
	else
	{
		WARNING_MSG(boost::format("DBInterfaceMysql::processException: "
				"Exception: %s\n") %
			dbe->what() );
	}

	return retry;
}

//-------------------------------------------------------------------------------------
}
