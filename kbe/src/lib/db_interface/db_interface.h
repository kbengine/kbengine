// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_DB_INTERFACE_H
#define KBE_DB_INTERFACE_H

#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "db_interface/entity_table.h"
#include "server/serverconfig.h"

namespace KBEngine { 

namespace thread
{
class ThreadPool;
}

class DBUtil;

/*
	数据库接口
*/
class DBInterface
{
public:
	enum DB_OP_STATE
	{
		DB_OP_READ,
		DB_OP_WRITE,
	};

	friend class DBUtil;

	DBInterface(const char* name) :
	db_port_(3306),
	db_numConnections_(1),
	lastquery_()
	{
		strncpy(name_, name, MAX_NAME);
		int dbIndex = g_kbeSrvConfig.dbInterfaceName2dbInterfaceIndex(this->name());
		KBE_ASSERT(dbIndex >= 0);
		dbIndex_ = dbIndex;
	};

	virtual ~DBInterface()
	{
	};

	/**
		检查环境
	*/
	virtual bool checkEnvironment() = 0;
	
	/**
		检查错误， 对错误的内容进行纠正
		如果纠正不成功返回失败
	*/
	virtual bool checkErrors() = 0;

	/**
		与某个数据库关联
	*/
	virtual bool attach(const char* databaseName = NULL) = 0;
	virtual bool detach() = 0;

	/**
		获取数据库所有的表名
	*/
	virtual bool getTableNames( std::vector<std::string>& tableNames, const char * pattern) = 0;

	/**
		获取数据库某个表所有的字段名称
	*/
	virtual bool getTableItemNames(const char* tableName, std::vector<std::string>& itemNames) = 0;

	/**
		查询表
	*/
	virtual bool query(const char* cmd, uint32 size, bool printlog = true, MemoryStream * result = NULL) = 0;
	virtual bool query(const std::string& cmd, bool printlog = true, MemoryStream * result = NULL)
	{
		return query(cmd.c_str(), (uint32)cmd.size(), printlog, result);
	}

	/**
		返回这个接口的名称
	*/
	const char* name() const { return name_; }

	/**
		返回这个接口的索引
	*/
	uint16 dbIndex() const { return dbIndex_; }

	/**
		返回这个接口的描述
	*/
	virtual const char* c_str() = 0;

	/** 
		获取错误
	*/
	virtual const char* getstrerror() = 0;

	/** 
		获取错误编号
	*/
	virtual int getlasterror() = 0;

	/**
		创建一个entity存储表
	*/
	virtual EntityTable* createEntityTable(EntityTables* pEntityTables) = 0;

	/** 
		从数据库删除entity表
	*/
	virtual bool dropEntityTableFromDB(const char* tableName) = 0;

	/** 
		从数据库删除entity表字段
	*/
	virtual bool dropEntityTableItemFromDB(const char* tableName, const char* tableItemName) = 0;

	/**
		锁住接口操作
	*/
	virtual bool lock() = 0;
	virtual bool unlock() = 0;

	/**
		处理异常
	*/
	virtual bool processException(std::exception & e) = 0;

	/**
		获取最后一次查询的sql语句
	*/
	virtual const std::string& lastquery() const{ return lastquery_; }

protected:
	char name_[MAX_BUF];									// 数据库接口的名称
	char db_type_[MAX_BUF];									// 数据库的类别
	uint32 db_port_;										// 数据库的端口
	char db_ip_[MAX_IP];									// 数据库的ip地址
	char db_username_[MAX_BUF];								// 数据库的用户名
	char db_password_[MAX_BUF * 10];						// 数据库的密码
	char db_name_[MAX_BUF];									// 数据库名
	uint16 db_numConnections_;								// 数据库最大连接
	std::string lastquery_;									// 最后一次查询描述
	uint16 dbIndex_;										// 对应的数据库接口索引
};

/*
	数据库操作单元
*/
class DBUtil : public Singleton<DBUtil>
{
public:
	DBUtil();
	~DBUtil();
	
	static bool initialize();
	static void finalise();
	static bool initializeWatcher();

	static bool initThread(const std::string& dbinterfaceName);
	static bool finiThread(const std::string& dbinterfaceName);

	static DBInterface* createInterface(const std::string& name, bool showinfo = true);
	static const char* accountScriptName();
	static bool initInterface(DBInterface* pdbi);

	static void handleMainTick();

	typedef KBEUnordered_map<std::string, thread::ThreadPool*> DBThreadPoolMap;
	static thread::ThreadPool* pThreadPool(const std::string& name)
	{ 
		DBThreadPoolMap::iterator iter = pThreadPoolMaps_.find(name);
		if (iter != pThreadPoolMaps_.end())
			return iter->second;

		return NULL;
	}

private:
	static DBThreadPoolMap pThreadPoolMaps_;
};

}

#endif // KBE_DB_INTERFACE_H
