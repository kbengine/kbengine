/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#ifndef KBE_READ_ENTITY_HELPER_H
#define KBE_READ_ENTITY_HELPER_H

// common include	
// #define NDEBUG
#include <sstream>
#include "common.h"
#include "sqlstatement.h"
#include "entity_sqlstatement_mapping.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "db_interface_mysql.h"

namespace KBEngine{ 

class ReadEntityHelper
{
public:
	ReadEntityHelper()
	{
	}

	virtual ~ReadEntityHelper()
	{
	}

	/**
		从表中查询数据
	*/
	static bool queryDB(DBInterface* pdbi, mysql::DBContext& context)
	{
		// 根据某个dbid获得一张表上的相关数据
		SqlStatement* pSqlcmd = new SqlStatementQuery(pdbi, context.tableName, 
			context.dbids[context.dbid], 
			context.dbid, context.items);

		bool ret = pSqlcmd->query();
		context.dbid = pSqlcmd->dbid();
		delete pSqlcmd;
		
		if(!ret)
			return ret;

		std::map<DBID, std::vector< std::string >, std::less<DBID> > resultsDatas;

		// 将查询到的结果写入上下文
		MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());

		if(pResult)
		{
			MYSQL_ROW arow;

			while((arow = mysql_fetch_row(pResult)) != NULL)
			{
				uint32 nfields = (uint32)mysql_num_fields(pResult);
				if(nfields <= 0)
					continue;

				unsigned long *lengths = mysql_fetch_lengths(pResult);

				// 查询命令保证了查询到的每条记录都会有dbid
				std::stringstream sval;
				sval << arow[0];
				DBID item_dbid;
				sval >> item_dbid;

				// 将dbid记录到列表中，如果当前表还存在子表引用则会去子表查每一条与此dbid相关的记录
				std::vector<DBID>& itemDBIDs = context.dbids[context.dbid];
				int fidx = -100;

				if (itemDBIDs.size() > 0 && itemDBIDs[itemDBIDs.size() - 1] > item_dbid)
				{
					for (fidx = itemDBIDs.size() - 1; fidx > 0; --fidx)
					{
						if (itemDBIDs[fidx] < item_dbid)
							break;
					}

					itemDBIDs.insert(itemDBIDs.begin() + fidx, item_dbid);
				}
				else
				{
					itemDBIDs.push_back(item_dbid);
				}

				// 如果这条记录除了dbid以外还存在其他数据，则将数据填充到结果集中
				if(nfields > 1)
				{
					KBE_ASSERT(nfields == context.items.size() + 1);
					for (uint32 i = 1; i < nfields; ++i)
					{
						KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA> pSotvs = context.items[i - 1];
						std::string data;
						data.assign(arow[i], lengths[i]);

						if (fidx != -100)
							resultsDatas[context.dbid].insert(resultsDatas[context.dbid].begin() + fidx++, data);
						else
							resultsDatas[context.dbid].push_back(data);
					}
				}
			}

			mysql_free_result(pResult);

			std::vector<DBID>::iterator diter = context.dbids[context.dbid].begin();
			for (; diter != context.dbids[context.dbid].end(); ++diter)
			{
				std::map< DBID, std::vector< std::string >, std::less<DBID> >::iterator friter = resultsDatas.find((*diter));
				if (friter == resultsDatas.end())
					continue;

				const std::vector< std::string >& resultsData = friter->second;
				context.results.insert(context.results.end(), resultsData.begin(), resultsData.end());
			}
		}
		
		std::vector<DBID>& dbids = context.dbids[context.dbid];

		// 如果没有数据则查询完毕了
		if(dbids.size() == 0)
			return true;

		// 如果当前表存在子表引用则需要继续查询子表
		// 每一个dbid都需要获得子表上的数据
		// 在这里我们让子表一次查询出所有的dbids数据然后填充到结果集

		mysql::DBContext::DB_RW_CONTEXTS::iterator iter1 = context.optable.begin();
		for(; iter1 != context.optable.end(); ++iter1)
		{
			mysql::DBContext& wbox = *iter1->second.get();
			if(!queryChildDB(pdbi, wbox, dbids))
				return false;
		}

		return ret;
	}


	/**
		从子表中查询数据
	*/
	static bool queryChildDB(DBInterface* pdbi, mysql::DBContext& context, std::vector<DBID>& parentTableDBIDs)
	{
		// 根据某个dbid获得一张表上的相关数据
		SqlStatement* pSqlcmd = new SqlStatementQuery(pdbi, context.tableName, 
			parentTableDBIDs, 
			context.dbid, context.items);

		bool ret = pSqlcmd->query();
		context.dbid = pSqlcmd->dbid();
		delete pSqlcmd;
		
		if(!ret)
			return ret;

		std::vector<DBID> t_parentTableDBIDs;
		std::map< DBID, std::vector< std::string >, std::less<DBID> > resultsDatas;
		
		// 将查询到的结果写入上下文
		MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());

		if(pResult)
		{
			MYSQL_ROW arow;

			while((arow = mysql_fetch_row(pResult)) != NULL)
			{
				uint32 nfields = (uint32)mysql_num_fields(pResult);
				if(nfields <= 0)
					continue;

				unsigned long *lengths = mysql_fetch_lengths(pResult);

				// 查询命令保证了查询到的每条记录都会有dbid
				std::stringstream sval;
				sval << arow[0];
				DBID item_dbid;
				sval >> item_dbid;

				sval.clear();
				sval << arow[1];
				DBID parentID;
				sval >> parentID;

				// 将dbid记录到列表中，如果当前表还存在子表引用则会去子表查每一条与此dbid相关的记录
				std::vector<DBID>& itemDBIDs = context.dbids[parentID];
				int fidx = -100;

				if (itemDBIDs.size() > 0 && itemDBIDs[itemDBIDs.size() - 1] > item_dbid)
				{
					for (fidx = itemDBIDs.size() - 1; fidx > 0; --fidx)
					{
						if (itemDBIDs[fidx] < item_dbid)
							break;
					}

					itemDBIDs.insert(itemDBIDs.begin() + fidx, item_dbid);
					t_parentTableDBIDs.insert(t_parentTableDBIDs.begin() + t_parentTableDBIDs.size() - (itemDBIDs.size() - fidx - 1), item_dbid);
				}
				else
				{
					itemDBIDs.push_back(item_dbid);
					t_parentTableDBIDs.push_back(item_dbid);
				}

				// 如果这条记录除了dbid以外还存在其他数据，则将数据填充到结果集中
				const uint32 const_fields = 2; // id, parentID
				if(nfields > const_fields)
				{
					KBE_ASSERT(nfields == context.items.size() + const_fields);
					for (uint32 i = const_fields; i < nfields; ++i)
					{
						KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA> pSotvs = context.items[i - const_fields];
						std::string data;
						data.assign(arow[i], lengths[i]);

						if (fidx != -100)
							resultsDatas[parentID].insert(resultsDatas[parentID].begin() + fidx++, data);
						else
							resultsDatas[parentID].push_back(data);
					}
				}
			}

			mysql_free_result(pResult);

			std::vector<DBID>::iterator diter = parentTableDBIDs.begin();
			for (; diter != parentTableDBIDs.end(); ++diter)
			{
				std::map< DBID, std::vector< std::string >, std::less<DBID> >::iterator friter = resultsDatas.find((*diter));
				if (friter == resultsDatas.end())
					continue;

				const std::vector< std::string >& resultsData = friter->second;
				context.results.insert(context.results.end(), resultsData.begin(), resultsData.end());
			}
		}

		// 如果没有数据则查询完毕了
		if(t_parentTableDBIDs.size() == 0)
			return true;

		// 如果当前表存在子表引用则需要继续查询子表
		// 每一个dbid都需要获得子表上的数据
		// 在这里我们让子表一次查询出所有的dbids数据然后填充到结果集
		mysql::DBContext::DB_RW_CONTEXTS::iterator iter1 = context.optable.begin();
		for(; iter1 != context.optable.end(); ++iter1)
		{
			mysql::DBContext& wbox = *iter1->second.get();

			if(!queryChildDB(pdbi, wbox, t_parentTableDBIDs))
				return false;
		}

		return ret;
	}

protected:
};

}
#endif // KBE_READ_ENTITY_HELPER_H
