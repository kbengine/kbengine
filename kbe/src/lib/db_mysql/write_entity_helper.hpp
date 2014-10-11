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

#ifndef KBE_WRITE_ENTITY_HELPER_HPP
#define KBE_WRITE_ENTITY_HELPER_HPP

// common include	
// #define NDEBUG
#include <sstream>
#include "common.hpp"
#include "sqlstatement.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/entity_table.hpp"
#include "db_interface_mysql.hpp"

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

	static SqlStatement* createSql(DBInterface* dbi, DB_TABLE_OP opType, 
		std::string tableName, DBID parentDBID, 
		DBID dbid, DB_OP_TABLE_ITEM_DATAS& tableVal)
	{
		SqlStatement* pSqlcmd = NULL;

		switch(opType)
		{
		case TABLE_OP_UPDATE:
			if(dbid > 0)
				pSqlcmd = new SqlStatementUpdate(dbi, tableName, parentDBID, dbid, tableVal);
			else
				pSqlcmd = new SqlStatementInsert(dbi, tableName, parentDBID, dbid, tableVal);
			break;
		case TABLE_OP_INSERT:
			pSqlcmd = new SqlStatementInsert(dbi, tableName, parentDBID, dbid, tableVal);
			break;
		case TABLE_OP_DELETE:
			break;
		default:
			KBE_ASSERT(false && "no support!\n");
		};

		return pSqlcmd;
	}

	/**
		�����ݸ��µ�����
	*/
	static bool writeDB(DB_TABLE_OP optype, DBInterface* dbi, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
	{
		bool ret = true;

		if(!opTableItemDataBox.isEmpty)
		{
			SqlStatement* pSqlcmd = createSql(dbi, optype, opTableItemDataBox.tableName, 
				opTableItemDataBox.parentTableDBID, 
				opTableItemDataBox.dbid, opTableItemDataBox.items);

			ret = pSqlcmd->query();
			opTableItemDataBox.dbid = pSqlcmd->dbid();
			delete pSqlcmd;
		}

		if(optype == TABLE_OP_INSERT)
		{
			// ��ʼ�������е��ӱ�
			DB_OP_TABLE_DATAS::iterator iter1 = opTableItemDataBox.optable.begin();
			for(; iter1 != opTableItemDataBox.optable.end(); iter1++)
			{
				DB_OP_TABLE_ITEM_DATA_BOX& wbox = *iter1->second.get();
				
				// �󶨱��ϵ
				wbox.parentTableDBID = opTableItemDataBox.dbid;

				// �����ӱ�
				writeDB(optype, dbi, wbox);
			}
		}
		else
		{
			// ����и�ID���ȵõ����������ݿ���ͬ��id�������ж�����Ŀ�� ��ȡ��ÿ�����ݵ�id
			// Ȼ���ڴ��е�����˳����������ݿ⣬ ������ݿ����д��ڵ���Ŀ��˳�򸲸Ǹ������е���Ŀ�� �����������
			// �������ݿ������е���Ŀ�����ʣ������ݣ� ��������������ݿ��е���Ŀ��ɾ�����ݿ��е���Ŀ
			// select id from tbl_SpawnPoint_xxx_values where parentID = 7;
			KBEUnordered_map< std::string, std::vector<DBID> > childTableDBIDs;

			if(opTableItemDataBox.dbid > 0)
			{
				DB_OP_TABLE_DATAS::iterator iter1 = opTableItemDataBox.optable.begin();
				for(; iter1 != opTableItemDataBox.optable.end(); iter1++)
				{
					DB_OP_TABLE_ITEM_DATA_BOX& wbox = *iter1->second.get();

					KBEUnordered_map<std::string, std::vector<DBID> >::iterator iter = 
						childTableDBIDs.find(opTableItemDataBox.tableName);

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
						kbe_snprintf(sqlstr, MAX_BUF * 10, "select count(id) from "ENTITY_TABLE_PERFIX"_%s where "TABLE_PARENTID_CONST_STR"=%"PRDBID" union all ", 
							tabiter->first.c_str(),
							opTableItemDataBox.dbid);
						
						sqlstr_getids += sqlstr;

						kbe_snprintf(sqlstr, MAX_BUF * 10, "select id from "ENTITY_TABLE_PERFIX"_%s where "TABLE_PARENTID_CONST_STR"=%"PRDBID, 
							tabiter->first.c_str(),
							opTableItemDataBox.dbid);

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
						kbe_snprintf(sqlstr, MAX_BUF * 10, "select id from "ENTITY_TABLE_PERFIX"_%s where "TABLE_PARENTID_CONST_STR"=%"PRDBID, 
							tabiter->first.c_str(),
							opTableItemDataBox.dbid);

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
			}

			// �����Ҫ��մ˱� ��ѭ��N���Ѿ��ҵ���dbid�� ʹ���ӱ��е��ӱ�Ҳ����Чɾ��
			if(!opTableItemDataBox.isEmpty)
			{
				// ��ʼ�������е��ӱ�
				DB_OP_TABLE_DATAS::iterator iter1 = opTableItemDataBox.optable.begin();
				for(; iter1 != opTableItemDataBox.optable.end(); iter1++)
				{
					DB_OP_TABLE_ITEM_DATA_BOX& wbox = *iter1->second.get();
					
					if(wbox.isEmpty)
						continue;

					// �󶨱��ϵ
					wbox.parentTableDBID = opTableItemDataBox.dbid;

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

					// �����ӱ�
					writeDB(optype, dbi, wbox);
				}
			}
			
			// ɾ��������������
			KBEUnordered_map< std::string, std::vector<DBID> >::iterator tabiter = childTableDBIDs.begin();
			for(; tabiter != childTableDBIDs.end(); tabiter++)
			{
				if(tabiter->second.size() == 0)
					continue;

				// ��ɾ�����ݿ��еļ�¼
				std::string sqlstr = "delete from "ENTITY_TABLE_PERFIX"_";
				sqlstr += tabiter->first;
				sqlstr += " where "TABLE_ID_CONST_STR" in (";

				std::vector<DBID>::iterator iter = tabiter->second.begin();
				for(; iter != tabiter->second.end(); iter++)
				{
					DBID dbid = (*iter);

					char sqlstr1[MAX_BUF];
					kbe_snprintf(sqlstr1, MAX_BUF, "%"PRDBID, dbid);
					sqlstr += sqlstr1;
					sqlstr += ",";
				}
				
				sqlstr.erase(sqlstr.size() - 1);
				sqlstr += ")";
				bool ret = dbi->query(sqlstr.c_str(), sqlstr.size(), false);
				KBE_ASSERT(ret);

				DB_OP_TABLE_DATAS::iterator iter1 = opTableItemDataBox.optable.begin();
				for(; iter1 != opTableItemDataBox.optable.end(); iter1++)
				{
					DB_OP_TABLE_ITEM_DATA_BOX& wbox = *iter1->second.get();
					if(wbox.tableName == tabiter->first)
					{
						std::vector<DBID>::iterator iter = tabiter->second.begin();
						for(; iter != tabiter->second.end(); iter++)
						{
							DBID dbid = (*iter);
							
							wbox.parentTableDBID = opTableItemDataBox.dbid;
							wbox.dbid = dbid;
							wbox.isEmpty = true;

							// �����ӱ�
							writeDB(optype, dbi, wbox);
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
#endif // KBE_WRITE_ENTITY_HELPER_HPP

