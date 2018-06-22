// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_WRITE_ENTITY_HELPER_H
#define KBE_WRITE_ENTITY_HELPER_H

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

class WriteEntityHelper
{
public:
	WriteEntityHelper()
	{
	}

	virtual ~WriteEntityHelper()
	{
	}

	static SqlStatement* createSql(DBInterface* pdbi, DB_TABLE_OP opType, 
		std::string tableName, DBID parentDBID, 
		DBID dbid, mysql::DBContext::DB_ITEM_DATAS& tableVal)
	{
		SqlStatement* pSqlcmd = NULL;

		switch(opType)
		{
		case TABLE_OP_UPDATE:
			if(dbid > 0)
				pSqlcmd = new SqlStatementUpdate(pdbi, tableName, parentDBID, dbid, tableVal);
			else
				pSqlcmd = new SqlStatementInsert(pdbi, tableName, parentDBID, dbid, tableVal);
			break;
		case TABLE_OP_INSERT:
			pSqlcmd = new SqlStatementInsert(pdbi, tableName, parentDBID, dbid, tableVal);
			break;
		case TABLE_OP_DELETE:
			break;
		default:
			KBE_ASSERT(false && "no support!\n");
		};

		return pSqlcmd;
	}

	/**
		将数据更新到表中
	*/
	static bool writeDB(DB_TABLE_OP optype, DBInterface* pdbi, mysql::DBContext& context)
	{
		bool ret = true;

		if(!context.isEmpty)
		{
			SqlStatement* pSqlcmd = createSql(pdbi, optype, context.tableName, 
				context.parentTableDBID, 
				context.dbid, context.items);

			ret = pSqlcmd->query();
			context.dbid = pSqlcmd->dbid();
			delete pSqlcmd;
		}

		if(optype == TABLE_OP_INSERT)
		{
			// 开始更新所有的子表
			mysql::DBContext::DB_RW_CONTEXTS::iterator iter1 = context.optable.begin();
			for(; iter1 != context.optable.end(); ++iter1)
			{
				mysql::DBContext& wbox = *iter1->second.get();
				
				// 绑定表关系
				wbox.parentTableDBID = context.dbid;

				// 更新子表
				writeDB(optype, pdbi, wbox);
			}
		}
		else
		{
			// 如果有父ID首先得到该属性数据库中同父id的数据有多少条目， 并取出每条数据的id
			// 然后将内存中的数据顺序更新至数据库， 如果数据库中有存在的条目则顺序覆盖更新已有的条目， 如果数据数量
			// 大于数据库中已有的条目则插入剩余的数据， 如果数据少于数据库中的条目则删除数据库中的条目
			// select id from tbl_SpawnPoint_xxx_values where parentID = 7;
			KBEUnordered_map< std::string, std::vector<DBID> > childTableDBIDs;

			if(context.dbid > 0)
			{
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
					
					if(pdbi->query(sqlstr_getids.c_str(), sqlstr_getids.size(), false))
					{
						MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
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

						if(pdbi->query(sqlstr, strlen(sqlstr), false))
						{
							MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
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
			}

			// 如果是要清空此表， 则循环N次已经找到的dbid， 使其子表中的子表也能有效删除
			if(!context.isEmpty)
			{
				// 开始更新所有的子表
				mysql::DBContext::DB_RW_CONTEXTS::iterator iter1 = context.optable.begin();
				for(; iter1 != context.optable.end(); ++iter1)
				{
					mysql::DBContext& wbox = *iter1->second.get();
					
					if(wbox.isEmpty)
						continue;

					// 绑定表关系
					wbox.parentTableDBID = context.dbid;

					KBEUnordered_map<std::string, std::vector<DBID> >::iterator iter = 
						childTableDBIDs.find(wbox.tableName);
					
					if(iter != childTableDBIDs.end())
					{
						if(iter->second.size() > 0)
						{
							wbox.dbid = iter->second.front();
							iter->second.erase(iter->second.begin());
						}

						if(iter->second.size() <= 0)
						{
							childTableDBIDs.erase(wbox.tableName);
						}
					}

					// 更新子表
					writeDB(optype, pdbi, wbox);
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
				bool ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), false);
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

							// 更新子表
							writeDB(optype, pdbi, wbox);
						}
					}
				}
			}
		}
		return ret;
	}

protected:

};

}
#endif // KBE_WRITE_ENTITY_HELPER_H

