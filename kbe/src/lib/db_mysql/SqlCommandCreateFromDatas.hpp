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

#ifndef __SQL_COMMAND_CREATE_FROM_DATAS_H__
#define __SQL_COMMAND_CREATE_FROM_DATAS_H__

// common include	
// #define NDEBUG
#include "common.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/entity_table.hpp"
#include "db_interface_mysql.hpp"

namespace KBEngine{ 

class SqlCommandCreateFromDatasBase
{
public:
	SqlCommandCreateFromDatasBase(std::string tableName, DBID parentDBID, DBID dbid, DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas):
	  tableItemDatas_(tableItemDatas),
	  sqlstr_(),
	  tableName_(tableName),
	  dbid_(dbid),
	  parentDBID_(parentDBID)
	{
	}

	virtual ~SqlCommandCreateFromDatasBase()
	{
	}

	std::string& sql(){ return sqlstr_; }

	virtual bool query(DBInterface* pdbi)
	{
		return static_cast<DBInterfaceMysql*>(pdbi)->query(sqlstr_.c_str(), sqlstr_.size(), false);
	}

	DBID dbid()const{ return dbid_; }
protected:
	DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas_;
	std::string sqlstr_;
	std::string tableName_;
	DBID dbid_;
	DBID parentDBID_;
};

class SqlCommandCreateFromDatas_INSERT : public SqlCommandCreateFromDatasBase
{
public:
	SqlCommandCreateFromDatas_INSERT(std::string tableName, DBID parentDBID, DBID dbid, DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas):
	  SqlCommandCreateFromDatasBase(tableName, parentDBID, dbid, tableItemDatas)
	{
		// insert into tbl_Account (sm_accountName) values("fdsafsad\0\fdsfasfsa\0fdsafsda");
		sqlstr_ = "insert into "ENTITY_TABLE_PERFIX"_";
		sqlstr_ += tableName;
		sqlstr_ += " (";
		sqlstr1_ = ")  values(";
		
		if(parentDBID > 0)
		{
			sqlstr_ += TABLE_PARENT_ID;
			sqlstr_ += ",";
			
			char strdbid[MAX_BUF];
			kbe_snprintf(strdbid, MAX_BUF, "%"PRDBID, parentDBID);
			sqlstr1_ += strdbid;
			sqlstr1_ += ",";
		}

		DB_W_OP_TABLE_ITEM_DATAS::iterator tableValIter = tableItemDatas.begin();
		for(; tableValIter != tableItemDatas.end(); tableValIter++)
		{
			std::tr1::shared_ptr<DB_W_OP_TABLE_ITEM_DATA> pSotvs = (*tableValIter);

			if(dbid > 0)
			{
			}
			else
			{
				sqlstr_ += pSotvs->sqlkey;
				if(pSotvs->extraDatas.size() > 0)
					sqlstr1_ += pSotvs->extraDatas;
				else
					sqlstr1_ += pSotvs->sqlval;

				sqlstr_ += ",";
				sqlstr1_ += ",";
			}
		}
		
		if(parentDBID > 0 || sqlstr_.at(sqlstr_.size() - 1) == ',')
			sqlstr_.erase(sqlstr_.size() - 1);

		if(parentDBID > 0 || sqlstr1_.at(sqlstr1_.size() - 1) == ',')
			sqlstr1_.erase(sqlstr1_.size() - 1);

		sqlstr1_ += ");";
		sqlstr_ += sqlstr1_;
	}

	virtual ~SqlCommandCreateFromDatas_INSERT()
	{
	}

	virtual bool query(DBInterface* pdbi)
	{
		bool ret = SqlCommandCreateFromDatasBase::query(pdbi);
		if(!ret)
		{
			ERROR_MSG("SqlCommandCreateFromDatas_INSERT::query: %s\n", pdbi->getstrerror());
			return false;
		}

		dbid_ = static_cast<DBInterfaceMysql*>(pdbi)->insertID();
		return ret;
	}

protected:
	
	std::string sqlstr1_;
};

class SqlCommandCreateFromDatas_UPDATE : public SqlCommandCreateFromDatasBase
{
public:
	SqlCommandCreateFromDatas_UPDATE(std::string tableName, DBID parentDBID, DBID dbid, DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas):
	  SqlCommandCreateFromDatasBase(tableName, parentDBID, dbid, tableItemDatas)
	{
		// update tbl_Account set sm_accountName="fdsafsad" where id=123;
		sqlstr_ = "update "ENTITY_TABLE_PERFIX"_";
		sqlstr_ += tableName;
		sqlstr_ += " set ";

		DB_W_OP_TABLE_ITEM_DATAS::iterator tableValIter = tableItemDatas.begin();
		for(; tableValIter != tableItemDatas.end(); tableValIter++)
		{
			std::tr1::shared_ptr<DB_W_OP_TABLE_ITEM_DATA> pSotvs = (*tableValIter);
			
			sqlstr_ += pSotvs->sqlkey;
			sqlstr_ += "=";
				
			if(pSotvs->extraDatas.size() > 0)
				sqlstr_ += pSotvs->extraDatas;
			else
				sqlstr_ += pSotvs->sqlval;

			sqlstr_ += ",";
		}

		if(sqlstr_.at(sqlstr_.size() - 1) == ',')
			sqlstr_.erase(sqlstr_.size() - 1);

		sqlstr_ += " where id=";
		
		char strdbid[MAX_BUF];
		kbe_snprintf(strdbid, MAX_BUF, "%"PRDBID, dbid);
		sqlstr_ += strdbid;
		sqlstr_ += ";";
	}

	virtual ~SqlCommandCreateFromDatas_UPDATE()
	{
	}
protected:
};

class SqlCommandHelper
{
public:
	SqlCommandHelper(std::string tableName, DBID parentDBID, DBID dbid, DB_W_OP_TABLE_ITEM_DATAS& tableVal)
	{
		if(dbid > 0)
			pSqlcmd_.reset(new SqlCommandCreateFromDatas_UPDATE(tableName, parentDBID, dbid, tableVal));
		else
			pSqlcmd_.reset(new SqlCommandCreateFromDatas_INSERT(tableName, parentDBID, dbid, tableVal));
	}

	virtual ~SqlCommandHelper()
	{
	}

	SqlCommandCreateFromDatasBase* operator ->()
	{
		return pSqlcmd_.get();
	}

	/**
		将数据更新到表中
	*/
	static bool updateTable(DBInterface* dbi, DB_W_OP_TABLE_ITEM_DATA_BOX opTableItemDataBox)
	{
		SqlCommandHelper sql(opTableItemDataBox.tableName, opTableItemDataBox.parentTableDBID, 
			opTableItemDataBox.dbid, opTableItemDataBox.items);

		sql->query(dbi);

		DBID dbid = sql->dbid();
		opTableItemDataBox.dbid = dbid;

		// 开始更新所有的子表
		DB_W_OP_TABLE_DATAS::iterator iter1 = opTableItemDataBox.optable.begin();
		for(; iter1 != opTableItemDataBox.optable.end(); iter1++)
		{
			DB_W_OP_TABLE_ITEM_DATA_BOX& wbox = *iter1->second.get();
			
			// 绑定表关系
			wbox.parentTableDBID = dbid;

			// 更新子表
			updateTable(dbi, wbox);
		}

		return true;
	}

protected:
	std::tr1::shared_ptr<SqlCommandCreateFromDatasBase> pSqlcmd_;
};

}
#endif
