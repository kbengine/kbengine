#pragma once
#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"

namespace KBEngine {
	namespace mongodb {
		class DBContext
		{
			public:
				/**
				存储所有要操作的表item结构
				*/
				struct DB_ITEM_DATA
				{
					char sqlval[MAX_BUF];
					const char* sqlkey;
					std::string extraDatas;
				};

				typedef std::vector< std::pair< std::string/*tableName*/, KBEShared_ptr< DBContext > > > DB_RW_CONTEXTS;
				typedef std::vector< KBEShared_ptr<DB_ITEM_DATA>  > DB_ITEM_DATAS;

				DBContext()
				{
				}

				~DBContext()
				{
				}

				DB_ITEM_DATAS items;

				std::string tableName;
				std::string parentTableName;

				DBID parentTableDBID;
				DBID dbid;

				DB_RW_CONTEXTS optable;

				bool isEmpty;

				std::map<DBID, std::vector<DBID> > dbids;
				std::vector< std::string >results;
				std::vector< std::string >::size_type readresultIdx;

			private:

		};

	}
}