// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_REMOVE_ENTITY_HELPER_H
#define KBE_REMOVE_ENTITY_HELPER_H

// common include	
// #define NDEBUG
#include <sstream>
#include "common.h"
#include "sqlstatement.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "db_interface_mysql.h"

namespace KBEngine{ 

class RemoveEntityHelper
{
public:
	RemoveEntityHelper()
	{
	}

	virtual ~RemoveEntityHelper()
	{
	}

	static bool removeDB(DBInterface* dbi, mysql::DBContext& context)
	{
		bool ret = _removeDB(dbi, context);

		if(!ret)
			return false;

		std::string sqlstr = "delete from " ENTITY_TABLE_PERFIX "_";
		sqlstr += context.tableName;
		sqlstr += " where " TABLE_ID_CONST_STR "=";

		char sqlstr1[MAX_BUF];
		kbe_snprintf(sqlstr1, MAX_BUF, "%" PRDBID, context.dbid);
		sqlstr += sqlstr1;
		
		ret = dbi->query(sqlstr.c_str(), sqlstr.size(), false);
		KBE_ASSERT(ret);

		return ret;
	}

	static bool _removeDB(DBInterface* dbi, mysql::DBContext& context)
	{
		bool ret = true;

		KBEUnordered_map< std::string, std::vector<DBID> > childTableDBIDs;

		mysql::DBContext::DB_RW_CONTEXTS::iterator iter1 = context.optable.begin();
		for(; iter1 != context.optable.end(); ++iter1)
		{
			mysql::DBContext& wbox = *iter1->second.get();

			KBEUnordered_map<std::string, std::vector<DBID> >::iterator iter = 
				childTableDBIDs.find(context.tableName);

			if(iter == childTableDBIDs.end())
			{
				std::vector<DBID> v;
				childTableDBIDs.insert(std::pair< std::string, std::vector<DBID> >(wbox.tableName, v));
			}
		}
		
		if(childTableDBIDs.size() > 1)
		{
			std::string sqlstr_getids;
			KBEUnordered_map< std::string, std::vector<DBID> >::iterator tabiter = childTableDBIDs.begin();
			for(; tabiter != childTableDBIDs.end();)
			{
				char sqlstr[MAX_BUF * 10];
				kbe_snprintf(sqlstr, MAX_BUF * 10, "select count(id) from " ENTITY_TABLE_PERFIX "_%s where " TABLE_PARENTID_CONST_STR "=%" PRDBID " union all ", 
					tabiter->first.c_str(),
					context.dbid);
				
				sqlstr_getids += sqlstr;

				kbe_snprintf(sqlstr, MAX_BUF * 10, "select id from " ENTITY_TABLE_PERFIX "_%s where " TABLE_PARENTID_CONST_STR "=%" PRDBID, 
					tabiter->first.c_str(),
					context.dbid);

				sqlstr_getids += sqlstr;
				if(++tabiter != childTableDBIDs.end())
					sqlstr_getids += " union all ";
			}
			
			if(dbi->query(sqlstr_getids.c_str(), sqlstr_getids.size(), false))
			{
				MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
				if(pResult)
				{
					MYSQL_ROW arow;
					int32 count = 0;
					tabiter = childTableDBIDs.begin();
					bool first = true;

					while((arow = mysql_fetch_row(pResult)) != NULL)
					{
						if(count == 0)
						{
							StringConv::str2value(count, arow[0]);
							if(!first || count <= 0)
								tabiter++;
							continue;
						}

						DBID old_dbid;
						StringConv::str2value(old_dbid, arow[0]);
						tabiter->second.push_back(old_dbid);
						count--;
						first = false;
					}

					mysql_free_result(pResult);
				}
			}
		}
		else if(childTableDBIDs.size() == 1)
		{
			KBEUnordered_map< std::string, std::vector<DBID> >::iterator tabiter = childTableDBIDs.begin();
				char sqlstr[MAX_BUF * 10];
				kbe_snprintf(sqlstr, MAX_BUF * 10, "select id from " ENTITY_TABLE_PERFIX "_%s where " TABLE_PARENTID_CONST_STR "=%" PRDBID, 
					tabiter->first.c_str(),
					context.dbid);

				if(dbi->query(sqlstr, strlen(sqlstr), false))
				{
					MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
					if(pResult)
					{
						MYSQL_ROW arow;
						while((arow = mysql_fetch_row(pResult)) != NULL)
						{
							DBID old_dbid;
							StringConv::str2value(old_dbid, arow[0]);
							tabiter->second.push_back(old_dbid);
						}

						mysql_free_result(pResult);
					}
				}
		}
	
		// 删除废弃的数据项
		KBEUnordered_map< std::string, std::vector<DBID> >::iterator tabiter = childTableDBIDs.begin();
		for(; tabiter != childTableDBIDs.end(); ++tabiter)
		{
			if(tabiter->second.size() == 0)
				continue;

			// 先删除数据库中的记录
			std::string sqlstr = "delete from " ENTITY_TABLE_PERFIX "_";
			sqlstr += tabiter->first;
			sqlstr += " where " TABLE_ID_CONST_STR " in (";

			std::vector<DBID>::iterator iter = tabiter->second.begin();
			for(; iter != tabiter->second.end(); ++iter)
			{
				DBID dbid = (*iter);

				char sqlstr1[MAX_BUF];
				kbe_snprintf(sqlstr1, MAX_BUF, "%" PRDBID, dbid);
				sqlstr += sqlstr1;
				sqlstr += ",";
			}
			
			sqlstr.erase(sqlstr.size() - 1);
			sqlstr += ")";
			bool ret = dbi->query(sqlstr.c_str(), sqlstr.size(), false);
			KBE_ASSERT(ret);

			mysql::DBContext::DB_RW_CONTEXTS::iterator iter1 = context.optable.begin();
			for(; iter1 != context.optable.end(); ++iter1)
			{
				mysql::DBContext& wbox = *iter1->second.get();
				if(wbox.tableName == tabiter->first)
				{
					std::vector<DBID>::iterator iter = tabiter->second.begin();
					for(; iter != tabiter->second.end(); ++iter)
					{
						DBID dbid = (*iter);
						
						wbox.parentTableDBID = context.dbid;
						wbox.dbid = dbid;
						wbox.isEmpty = true;

						_removeDB(dbi, wbox);
					}
				}
			}
		}

		return ret;
	}

protected:

};

}
#endif // KBE_REMOVE_ENTITY_HELPER_H
