/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef KBE_REDIS_HELPER_H
#define KBE_REDIS_HELPER_H

#include "common.h"
#include "common/common.h"
#include "common/stringconv.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "db_interface_redis.h"

namespace KBEngine{ 

class RedisHelper
{
public:
	RedisHelper()
	{
	}

	virtual ~RedisHelper()
	{
	}

	static bool expireKey(DBInterfaceRedis* pdbi, const std::string& key, int secs, bool printlog = true)
	{
		if (!pdbi->query(printlog, "EXPIRE %s %d", key.c_str(), secs))
			return false;
	}
	
	static bool check_array_results(redisReply* pRedisReply)
	{
		for(size_t j = 0; j < pRedisReply->elements; ++j) 
		{
			if(pRedisReply->element[j]->type != REDIS_REPLY_INTEGER && 
				pRedisReply->element[j]->type != REDIS_REPLY_STRING)
			{
				return false;
			}
		}
		
		return true;
	}
	
	static bool hasTable(DBInterfaceRedis* pdbi, const std::string& name, bool printlog = true)
	{
		redisReply* pRedisReply = NULL;
		
		if (!pdbi->query(fmt::format("scan 0 MATCH {}", name), &pRedisReply, printlog))
			return false;
		
		size_t size = 0;
		
		if(pRedisReply)
		{
			if(pRedisReply->elements == 2 && pRedisReply->element[1]->type == REDIS_REPLY_ARRAY)
			{
				size = pRedisReply->element[1]->elements;
			}
			
			freeReplyObject(pRedisReply); 
		}
		
		return size > 0;
	}
	
	static bool dropTable(DBInterfaceRedis* pdbi, const std::string& tableName, bool printlog = true)
	{
		uint64 index = 0;
		
		while(true)
		{
			redisReply* pRedisReply = NULL;
			
			pdbi->query(fmt::format("scan {} MATCH {}", index, tableName), &pRedisReply, printlog);
			
			if(pRedisReply)
			{
				if(pRedisReply->elements == 2)
				{
					KBE_ASSERT(pRedisReply->element[0]->type == REDIS_REPLY_STRING);
					
					// ��һ�������index��ʼ
					StringConv::str2value(index, pRedisReply->element[0]->str);
					
					redisReply* r0 = pRedisReply->element[1];
					KBE_ASSERT(r0->type == REDIS_REPLY_ARRAY);
					
					for(size_t j = 0; j < r0->elements; ++j) 
					{
						redisReply* r1 = r0->element[j];
						KBE_ASSERT(r1->type == REDIS_REPLY_STRING);
							
						pdbi->query(fmt::format("del {}", r1->str), &pRedisReply, printlog);
					}
				}
				
				freeReplyObject(pRedisReply); 
			}
			else
			{
				return false;
			}
			
			if(index == 0)
				break;
		}
		
		return true;
	}
	
	static bool dropTableItem(DBInterfaceRedis* pdbi, const std::string& tableName, 
		const std::string& itemName, bool printlog = true)
	{
		uint64 index = 0;
		
		while(true)
		{
			redisReply* pRedisReply = NULL;
			
			pdbi->query(fmt::format("scan {} MATCH {}", index, tableName), &pRedisReply, printlog);
			
			if(pRedisReply)
			{
				if(pRedisReply->elements == 2)
				{
					KBE_ASSERT(pRedisReply->element[0]->type == REDIS_REPLY_STRING);
					
					// ��һ�������index��ʼ
					StringConv::str2value(index, pRedisReply->element[0]->str);
					
					redisReply* r0 = pRedisReply->element[1];
					KBE_ASSERT(r0->type == REDIS_REPLY_ARRAY);
					
					for(size_t j = 0; j < r0->elements; ++j) 
					{
						redisReply* r1 = r0->element[j];
						KBE_ASSERT(r1->type == REDIS_REPLY_STRING);

						pdbi->query(fmt::format("hdel {} {}", r1->str, itemName), &pRedisReply, printlog);
					}
				}
				
				freeReplyObject(pRedisReply); 
			}
			else
			{
				return false;
			}
			
			if(index == 0)
				break;
		}
		
		return true;
	}
};

}
#endif // KBE_REDIS_HELPER_H
