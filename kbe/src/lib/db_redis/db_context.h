// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_REDIS_DB_RW_CONTEXT_H
#define KBE_REDIS_DB_RW_CONTEXT_H

#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"

namespace KBEngine { 
namespace redis { 

/**
	读写删操作时会用到，包含取到或待写入的各种信息。

	dbid：如果是主表就是实体的dbid，子表就是当前查询的dbid

	dbids：主表上dbids中只有一个dbid，就是实体的id，如果实体数据存在数组类则会有子表出现，当这个数据结构描述的是一个子表的时候
		dbids是这个数组的子表索引, 每个dbid都表示这个子表上对应的值并且按照排列顺序同时也表示在数组中对应位置的值。
		dbids = {
			123 : [xxx, xxx, ...], // 123为父表上的某个dbid，数组为在子表上与父表相关联的的dbids。
			...
		}

	items：中有这个表的字段信息，如果是写库则字段中也有对应的要写值。

	optable：子表结构

	results：读操作时查询到的数据, 数据的排列对应items中的strkey的数量乘以dbids的数量。
	readresultIdx：因为results中的数量是dbids * items，所以当在某些递归读的时候填充数据会根据这个readresultIdx计算填充的位置。

	parentTableDBID：父表的dbid
	parentTableName：父表的名称

	tableName：当前表的名称
 */
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
#endif // KBE_REDIS_DB_RW_CONTEXT_H

