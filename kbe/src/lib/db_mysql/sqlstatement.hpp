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

#ifndef __SQL_STATEMENT_H__
#define __SQL_STATEMENT_H__

// common include	
// #define NDEBUG
#include <sstream>
#include "common.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/entity_table.hpp"
#include "db_interface_mysql.hpp"

namespace KBEngine{ 

class SqlStatement
{
public:
	SqlStatement(DBInterface* dbi, std::string tableName, DBID parentDBID, DBID dbid, 
		DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas):
	  tableItemDatas_(tableItemDatas),
	  sqlstr_(),
	  tableName_(tableName),
	  dbid_(dbid),
	  parentDBID_(parentDBID),
	  dbi_(dbi)
	{
	}

	virtual ~SqlStatement()
	{
	}

	std::string& sql(){ return sqlstr_; }

	virtual bool query()
	{
		// 没有数据更新
		if(sqlstr_ == "")
			return true;

		return static_cast<DBInterfaceMysql*>(dbi_)->query(sqlstr_.c_str(), sqlstr_.size(), false);
	}

	DBID dbid()const{ return dbid_; }
protected:
	DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas_;
	std::string sqlstr_;
	std::string tableName_;
	DBID dbid_;
	DBID parentDBID_;
	DBInterface* dbi_; 
};

class SqlStatementInsert : public SqlStatement
{
public:
	SqlStatementInsert(DBInterface* dbi, std::string tableName, DBID parentDBID, 
		DBID dbid, DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas):
	  SqlStatement(dbi, tableName, parentDBID, dbid, tableItemDatas)
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

	virtual ~SqlStatementInsert()
	{
	}

	virtual bool query()
	{
		// 没有数据更新
		if(sqlstr_ == "")
			return true;

		bool ret = SqlStatement::query();
		if(!ret)
		{
			ERROR_MSG("SqlStatementInsert::query: %s\n", dbi_->getstrerror());
			return false;
		}

		dbid_ = static_cast<DBInterfaceMysql*>(dbi_)->insertID();
		return ret;
	}

protected:
	
	std::string sqlstr1_;
};

class SqlStatementUpdate : public SqlStatement
{
public:
	SqlStatementUpdate(DBInterface* dbi, std::string tableName, DBID parentDBID, 
		DBID dbid, DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas):
	  SqlStatement(dbi, tableName, parentDBID, dbid, tableItemDatas)
	{
		if(tableItemDatas.size() == 0)
		{
			sqlstr_ = "";
			return;
		}

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

	virtual ~SqlStatementUpdate()
	{
	}
protected:
};

class SqlStatementQuery : public SqlStatement
{
public:
	SqlStatementQuery(DBInterface* dbi, std::string tableName, DBID parentDBID, 
		DBID dbid, DB_W_OP_TABLE_ITEM_DATAS& tableItemDatas):
	  SqlStatement(dbi, tableName, parentDBID, dbid, tableItemDatas)
	{
		if(tableItemDatas.size() == 0)
		{
			sqlstr_ = "";
			return;
		}

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

	virtual ~SqlStatementQuery()
	{
	}
protected:
};

}
#endif
