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

#include "redis_watcher.h"
#include "common.h"
#include "common/common.h"
#include "helper/watcher.h"
#include "thread/threadguard.h"
#include "thread/threadmutex.h"
#include "server/serverconfig.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"

namespace KBEngine{ 

static KBEngine::thread::ThreadMutex _g_logMutex;
static KBEUnordered_map< std::string, uint32 > g_querystatistics;
static bool _g_installedWatcher = false;
static bool _g_debug = false;

//-------------------------------------------------------------------------------------
static uint32 watcher_query(std::string cmd)
{
	KBEngine::thread::ThreadGuard tg(&_g_logMutex);

	KBEUnordered_map< std::string, uint32 >::iterator iter = g_querystatistics.find(cmd);
	if (iter != g_querystatistics.end())
	{
		return iter->second;
	}

	return 0;
}

//-------------------------------------------------------------------------------------
static uint32 watcher_PING()
{
	return watcher_query("PING");
}

static uint32 watcher_SCAN()
{
	return watcher_query("SCAN");
}

// STRING
static uint32 watcher_SET()
{
	return watcher_query("SET");
}

static uint32 watcher_GET()
{
	return watcher_query("GET");
}

static uint32 watcher_GETRANGE()
{
	return watcher_query("GETRANGE");
}

static uint32 watcher_GETSET()
{
	return watcher_query("GETSET");
}

static uint32 watcher_GETBIT()
{
	return watcher_query("GETBIT");
}

static uint32 watcher_MGET()
{
	return watcher_query("MGET");
}

static uint32 watcher_SETBIT()
{
	return watcher_query("SETBIT");
}

static uint32 watcher_SETEX()
{
	return watcher_query("SETEX");
}

static uint32 watcher_SETNX()
{
	return watcher_query("SETNX");
}

static uint32 watcher_SETRANGE()
{
	return watcher_query("SETRANGE");
}

static uint32 watcher_STRLEN()
{
	return watcher_query("STRLEN");
}

static uint32 watcher_MSET()
{
	return watcher_query("MSET");
}

static uint32 watcher_MSETNX()
{
	return watcher_query("MSETNX");
}

static uint32 watcher_PSETEX()
{
	return watcher_query("PSETEX");
}

static uint32 watcher_INCR()
{
	return watcher_query("INCR");
}

static uint32 watcher_INCRBY()
{
	return watcher_query("INCRBY");
}

// KEY
static uint32 watcher_DEL()
{
	return watcher_query("DEL");
}

static uint32 watcher_DUMP()
{
	return watcher_query("DUMP");
}

static uint32 watcher_EXISTS()
{
	return watcher_query("EXISTS");
}

static uint32 watcher_EXPIRE()
{
	return watcher_query("EXPIRE");
}

static uint32 watcher_EXPIREAT()
{
	return watcher_query("EXPIREAT");
}

static uint32 watcher_PEXPIRE()
{
	return watcher_query("PEXPIRE");
}

static uint32 watcher_PEXPIREAT()
{
	return watcher_query("PEXPIREAT");
}

static uint32 watcher_KEYS()
{
	return watcher_query("KEYS");
}

static uint32 watcher_MOVE()
{
	return watcher_query("MOVE");
}

static uint32 watcher_PERSIST()
{
	return watcher_query("PERSIST");
}

static uint32 watcher_PTTL()
{
	return watcher_query("PTTL");
}

static uint32 watcher_TTL()
{
	return watcher_query("TTL");
}

static uint32 watcher_RANDOMKEY()
{
	return watcher_query("RANDOMKEY");
}

static uint32 watcher_RENAME()
{
	return watcher_query("RENAME");
}

static uint32 watcher_RENAMENX()
{
	return watcher_query("RENAMENX");
}

static uint32 watcher_TYPE()
{
	return watcher_query("TYPE");
}

// HASHES
static uint32 watcher_HDEL()
{
	return watcher_query("HDEL");
}

static uint32 watcher_HEXISTS()
{
	return watcher_query("HEXISTS");
}

static uint32 watcher_HGET()
{
	return watcher_query("HGET");
}

static uint32 watcher_HGETALL()
{
	return watcher_query("HGETALL");
}

static uint32 watcher_HINCRBY()
{
	return watcher_query("HINCRBY");
}

static uint32 watcher_HINCRBYFLOAT()
{
	return watcher_query("HINCRBYFLOAT");
}

static uint32 watcher_HKEYS()
{
	return watcher_query("HKEYS");
}

static uint32 watcher_HLEN()
{
	return watcher_query("HLEN");
}

static uint32 watcher_HMGET()
{
	return watcher_query("HMGET");
}

static uint32 watcher_HMSET()
{
	return watcher_query("HMSET");
}

static uint32 watcher_HSET()
{
	return watcher_query("HSET");
}

static uint32 watcher_HSETNX()
{
	return watcher_query("HSETNX");
}

static uint32 watcher_HVALS()
{
	return watcher_query("HVALS");
}

static uint32 watcher_HSCAN()
{
	return watcher_query("HSCAN");
}

//-------------------------------------------------------------------------------------
void RedisWatcher::initializeWatcher()
{
	if(_g_installedWatcher)
		return;

	_g_installedWatcher = true;
	_g_debug = g_kbeSrvConfig.getDBMgr().debugDBMgr;
	
	WATCH_OBJECT("db_querys/PING", &KBEngine::watcher_PING);
	WATCH_OBJECT("db_querys/SCAN", &KBEngine::watcher_SCAN);
	WATCH_OBJECT("db_querys/SET", &KBEngine::watcher_SET);
	WATCH_OBJECT("db_querys/GET", &KBEngine::watcher_GET);
	WATCH_OBJECT("db_querys/GETRANGE", &KBEngine::watcher_GETRANGE);
	WATCH_OBJECT("db_querys/GETSET", &KBEngine::watcher_GETSET);
	WATCH_OBJECT("db_querys/GETBIT", &KBEngine::watcher_GETBIT);
	WATCH_OBJECT("db_querys/MGET", &KBEngine::watcher_MGET);
	WATCH_OBJECT("db_querys/SETBIT", &KBEngine::watcher_SETBIT);
	WATCH_OBJECT("db_querys/SETEX", &KBEngine::watcher_SETEX);
	WATCH_OBJECT("db_querys/SETNX", &KBEngine::watcher_SETNX);
	WATCH_OBJECT("db_querys/SETRANGE", &KBEngine::watcher_SETRANGE);
	WATCH_OBJECT("db_querys/STRLEN", &KBEngine::watcher_STRLEN);
	WATCH_OBJECT("db_querys/MSET", &KBEngine::watcher_MSET);
	WATCH_OBJECT("db_querys/MSETNX", &KBEngine::watcher_MSETNX);
	WATCH_OBJECT("db_querys/PSETEX", &KBEngine::watcher_PSETEX);
	WATCH_OBJECT("db_querys/INCR", &KBEngine::watcher_INCR);
	WATCH_OBJECT("db_querys/INCRBY", &KBEngine::watcher_INCRBY);
	WATCH_OBJECT("db_querys/DEL", &KBEngine::watcher_DEL);
	WATCH_OBJECT("db_querys/DUMP", &KBEngine::watcher_DUMP);
	WATCH_OBJECT("db_querys/EXISTS", &KBEngine::watcher_EXISTS);
	WATCH_OBJECT("db_querys/EXPIRE", &KBEngine::watcher_EXPIRE);
	WATCH_OBJECT("db_querys/EXPIREAT", &KBEngine::watcher_EXPIREAT);
	WATCH_OBJECT("db_querys/PEXPIRE", &KBEngine::watcher_PEXPIRE);
	WATCH_OBJECT("db_querys/PEXPIREAT", &KBEngine::watcher_PEXPIREAT);
	WATCH_OBJECT("db_querys/KEYS", &KBEngine::watcher_KEYS);
	WATCH_OBJECT("db_querys/MOVE", &KBEngine::watcher_MOVE);
	WATCH_OBJECT("db_querys/PERSIST", &KBEngine::watcher_PERSIST);
	WATCH_OBJECT("db_querys/PTTL", &KBEngine::watcher_PTTL);
	WATCH_OBJECT("db_querys/TTL", &KBEngine::watcher_TTL);	
	WATCH_OBJECT("db_querys/RANDOMKEY", &KBEngine::watcher_RANDOMKEY);
	WATCH_OBJECT("db_querys/RENAME", &KBEngine::watcher_RENAME);
	WATCH_OBJECT("db_querys/RENAMENX", &KBEngine::watcher_RENAMENX);
	WATCH_OBJECT("db_querys/TYPE", &KBEngine::watcher_TYPE);
	WATCH_OBJECT("db_querys/HDEL", &KBEngine::watcher_HDEL);
	WATCH_OBJECT("db_querys/HEXISTS", &KBEngine::watcher_HEXISTS);
	WATCH_OBJECT("db_querys/HGET", &KBEngine::watcher_HGET);
	WATCH_OBJECT("db_querys/HGETALL", &KBEngine::watcher_HGETALL);
	WATCH_OBJECT("db_querys/HINCRBY", &KBEngine::watcher_HINCRBY);
	WATCH_OBJECT("db_querys/HINCRBYFLOAT", &KBEngine::watcher_HINCRBYFLOAT);
	WATCH_OBJECT("db_querys/HKEYS", &KBEngine::watcher_HKEYS);
	WATCH_OBJECT("db_querys/HLEN", &KBEngine::watcher_HLEN);
	WATCH_OBJECT("db_querys/HMGET", &KBEngine::watcher_HMGET);
	WATCH_OBJECT("db_querys/HMSET", &KBEngine::watcher_HMSET);
	WATCH_OBJECT("db_querys/HSET", &KBEngine::watcher_HSET);
	WATCH_OBJECT("db_querys/HSETNX", &KBEngine::watcher_HSETNX);
	WATCH_OBJECT("db_querys/HVALS", &KBEngine::watcher_HVALS);
	WATCH_OBJECT("db_querys/HSCAN", &KBEngine::watcher_HSCAN);	
}

//-------------------------------------------------------------------------------------
void RedisWatcher::querystatistics(const char* strCommand, uint32 size)
{
	std::string op;
	for(uint32 i=0; i<size; ++i)
	{
		if(strCommand[i] == ' ')
			break;

		op += strCommand[i];
	}

	if(op.size() == 0)
		return;

	std::transform(op.begin(), op.end(), op.begin(), toupper);

	_g_logMutex.lockMutex();

	KBEUnordered_map< std::string, uint32 >::iterator iter = g_querystatistics.find(op);
	if(iter == g_querystatistics.end())
	{
		g_querystatistics[op] = 1;
	}
	else
	{
		iter->second += 1;
	}

	_g_logMutex.unlockMutex();
}

//-------------------------------------------------------------------------------------

}

