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

#ifndef __READ_ENTITY_HELPER_H__
#define __READ_ENTITY_HELPER_H__

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
	static bool queryDB(DBInterface* dbi, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
	{
		SqlStatement* pSqlcmd = new SqlStatementQuery(dbi, opTableItemDataBox.tableName, 
			opTableItemDataBox.parentTableDBID, 
			opTableItemDataBox.dbid, opTableItemDataBox.items);

		bool ret = pSqlcmd->query();
		opTableItemDataBox.dbid = pSqlcmd->dbid();
		delete pSqlcmd;
		
		if(!ret)
			return ret;

		MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());

		if(pResult)
		{
			MYSQL_ROW arow;

			while((arow = mysql_fetch_row(pResult)) != NULL)
			{
				uint32 nfields = (uint32)mysql_num_fields(pResult);
				if(nfields <= 0)
					continue;

				unsigned long *lengths = mysql_fetch_lengths(pResult);

				std::stringstream sval;
				sval << arow[0];
				DBID item_dbid;
				
				sval >> item_dbid;
				opTableItemDataBox.dbids[opTableItemDataBox.parentTableDBID > 0 ? 
							opTableItemDataBox.parentTableDBID : opTableItemDataBox.dbid].push_back(item_dbid);

				if(nfields > 1)
				{
					KBE_ASSERT(nfields == opTableItemDataBox.items.size() + 1);
					for (uint32 i = 1; i < nfields; i++)
					{
						std::tr1::shared_ptr<DB_OP_TABLE_ITEM_DATA> pSotvs = opTableItemDataBox.items[i - 1];
						std::string data;
						data.assign(arow[i], lengths[i]);

						opTableItemDataBox.results[opTableItemDataBox.parentTableDBID > 0 ? 
							opTableItemDataBox.parentTableDBID : opTableItemDataBox.dbid].push_back(data);
					}
				}
			}

			mysql_free_result(pResult);
		}
		
		std::vector<DBID>& dbids = opTableItemDataBox.dbids[opTableItemDataBox.parentTableDBID > 0 ? 
							opTableItemDataBox.parentTableDBID : opTableItemDataBox.dbid];

		if(dbids.size() == 0)
			return true;

		std::vector<DBID>::iterator dbidIter = dbids.begin();
		for(; dbidIter != dbids.end(); dbidIter++)
		{
			DB_OP_TABLE_DATAS::iterator iter1 = opTableItemDataBox.optable.begin();
			for(; iter1 != opTableItemDataBox.optable.end(); iter1++)
			{
				DB_OP_TABLE_ITEM_DATA_BOX& wbox = *iter1->second.get();
				
				// 绑定表关系
				wbox.parentTableDBID = (*dbidIter);

				if(!queryDB(dbi, wbox))
					return false;
			}
		}

		return ret;
	}

protected:
};

}
#endif
