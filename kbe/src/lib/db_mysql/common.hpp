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

#ifndef KBE_DB_SQL_COMMON_HPP
#define KBE_DB_SQL_COMMON_HPP

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine{ 

/**
	db�������ݽṹ
*/
struct DB_OP_TABLE_ITEM_DATA;

/**
	�洢����Ҫ�����ı�item�ṹ
*/
typedef std::vector< KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>  > DB_OP_TABLE_ITEM_DATAS;

/**
	�洢Ҫ�����ı����������
*/
struct DB_OP_TABLE_ITEM_DATA_BOX;

/**
	����Ҫ�����ı����ݽṹ
*/
typedef std::vector< std::pair< std::string/*tableName*/, KBEShared_ptr< DB_OP_TABLE_ITEM_DATA_BOX > > > DB_OP_TABLE_DATAS;

struct DB_OP_TABLE_ITEM_DATA
{
	char sqlval[MAX_BUF];
	const char* sqlkey;
	std::string extraDatas;
};

struct DB_OP_TABLE_ITEM_DATA_BOX
{
	DB_OP_TABLE_ITEM_DATAS items;
	std::string tableName;
	std::string parentTableName;
	DBID parentTableDBID;
	DBID dbid;
	DB_OP_TABLE_DATAS optable;
	bool isEmpty;
	std::map<DBID, std::vector<DBID> > dbids;
	std::vector< std::string >results;
	std::vector< std::string >::size_type readresultIdx;
};

}
#endif // KBE_DB_SQL_COMMON_HPP
