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

#ifndef __DB_SQL_COMMON_H__
#define __DB_SQL_COMMON_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine{ 

/**
	db操作数据结构
*/
struct DB_OP_TABLE_ITEM_DATA;

/**
	存储所有要操作的表item结构
*/
typedef std::vector< std::tr1::shared_ptr<DB_OP_TABLE_ITEM_DATA>  > DB_OP_TABLE_ITEM_DATAS;

/**
	存储要操作的表的所有内容
*/
struct DB_OP_TABLE_ITEM_DATA_BOX;

/**
	所有要操作的表数据结构
*/
typedef std::vector< std::pair< std::string/*tableName*/, std::tr1::shared_ptr< DB_OP_TABLE_ITEM_DATA_BOX > > > DB_OP_TABLE_DATAS;

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
#endif
