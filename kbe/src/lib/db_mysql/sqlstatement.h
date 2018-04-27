// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SQL_STATEMENT_H
#define KBE_SQL_STATEMENT_H

// common include	
// #define NDEBUG
#include <sstream>
#include "common.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "db_interface_mysql.h"

namespace KBEngine{ 

class SqlStatement
{
public:
	SqlStatement(DBInterface* pdbi, std::string tableName, DBID parentDBID, DBID dbid, 
		mysql::DBContext::DB_ITEM_DATAS& tableItemDatas) :
	  tableItemDatas_(tableItemDatas),
	  sqlstr_(),
	  tableName_(tableName),
	  dbid_(dbid),
	  parentDBID_(parentDBID),
	  pdbi_(pdbi)
	{
	}

	virtual ~SqlStatement()
	{
	}

	std::string& sql(){ return sqlstr_; }

	virtual bool query(DBInterface* pdbi = NULL)
	{
		// 没有数据更新
		if(sqlstr_ == "")
			return true;

		bool ret = static_cast<DBInterfaceMysql*>(pdbi != NULL ? pdbi : pdbi_)->query(sqlstr_.c_str(), sqlstr_.size(), false);

		if(!ret)
		{
			ERROR_MSG(fmt::format("SqlStatement::query: {}\n\tsql:{}\n", 
				(pdbi != NULL ? pdbi : pdbi_)->getstrerror(), sqlstr_));

			return false;
		}

		return ret;
	}

	DBID dbid() const{ return dbid_; }

protected:
	mysql::DBContext::DB_ITEM_DATAS& tableItemDatas_;
	std::string sqlstr_;
	std::string tableName_;
	DBID dbid_;
	DBID parentDBID_;
	DBInterface* pdbi_; 
};

class SqlStatementInsert : public SqlStatement
{
public:
	SqlStatementInsert(DBInterface* pdbi, std::string tableName, DBID parentDBID, 
		DBID dbid, mysql::DBContext::DB_ITEM_DATAS& tableItemDatas) :
	  SqlStatement(pdbi, tableName, parentDBID, dbid, tableItemDatas)
	{
		// insert into tbl_Account (sm_accountName) values("fdsafsad\0\fdsfasfsa\0fdsafsda");
		sqlstr_ = "insert into " ENTITY_TABLE_PERFIX "_";
		sqlstr_ += tableName;
		sqlstr_ += " (";
		sqlstr1_ = ")  values(";
		
		if(parentDBID > 0)
		{
			sqlstr_ += TABLE_PARENTID_CONST_STR;
			sqlstr_ += ",";
			
			char strdbid[MAX_BUF];
			kbe_snprintf(strdbid, MAX_BUF, "%" PRDBID, parentDBID);
			sqlstr1_ += strdbid;
			sqlstr1_ += ",";
		}

		mysql::DBContext::DB_ITEM_DATAS::iterator tableValIter = tableItemDatas.begin();
		for(; tableValIter != tableItemDatas.end(); ++tableValIter)
		{
			KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA> pSotvs = (*tableValIter);

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

		sqlstr1_ += ")";
		sqlstr_ += sqlstr1_;
	}

	virtual ~SqlStatementInsert()
	{
	}

	virtual bool query(DBInterface* pdbi = NULL)
	{
		// 没有数据更新
		if(sqlstr_ == "")
			return true;

		bool ret = SqlStatement::query(pdbi);
		if(!ret)
		{
			ERROR_MSG(fmt::format("SqlStatementInsert::query: {}\n\tsql:{}\n",
				(pdbi != NULL ? pdbi : pdbi_)->getstrerror(), sqlstr_));

			return false;
		}

		dbid_ = static_cast<DBInterfaceMysql*>(pdbi != NULL ? pdbi : pdbi_)->insertID();
		return ret;
	}

protected:
	std::string sqlstr1_;
};

class SqlStatementUpdate : public SqlStatement
{
public:
	SqlStatementUpdate(DBInterface* pdbi, std::string tableName, DBID parentDBID, 
		DBID dbid, mysql::DBContext::DB_ITEM_DATAS& tableItemDatas) :
	  SqlStatement(pdbi, tableName, parentDBID, dbid, tableItemDatas)
	{
		if(tableItemDatas.size() == 0)
		{
			sqlstr_ = "";
			return;
		}

		// update tbl_Account set sm_accountName="fdsafsad" where id=123;
		sqlstr_ = "update " ENTITY_TABLE_PERFIX "_";
		sqlstr_ += tableName;
		sqlstr_ += " set ";

		mysql::DBContext::DB_ITEM_DATAS::iterator tableValIter = tableItemDatas.begin();
		for(; tableValIter != tableItemDatas.end(); ++tableValIter)
		{
			KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA> pSotvs = (*tableValIter);
			
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
		kbe_snprintf(strdbid, MAX_BUF, "%" PRDBID, dbid);
		sqlstr_ += strdbid;
	}

	virtual ~SqlStatementUpdate()
	{
	}

protected:
};

class SqlStatementQuery : public SqlStatement
{
public:
	SqlStatementQuery(DBInterface* pdbi, std::string tableName, const std::vector<DBID>& parentTableDBIDs, 
		DBID dbid, mysql::DBContext::DB_ITEM_DATAS& tableItemDatas) :
	  SqlStatement(pdbi, tableName, 0, dbid, tableItemDatas),
	  sqlstr1_()
	{

		// select id,xxx from tbl_SpawnPoint where id=123;
		sqlstr_ = "select id,";
		// 无论哪种情况都查询出ID字段
		sqlstr1_ += " from " ENTITY_TABLE_PERFIX "_";
		sqlstr1_ += tableName;
		
		char strdbid[MAX_BUF];

		if(parentTableDBIDs.size() == 0)
		{
			sqlstr1_ += " where id=";
			kbe_snprintf(strdbid, MAX_BUF, "%" PRDBID, dbid);
			sqlstr1_ += strdbid;
		}
		else
		{
			sqlstr_ += TABLE_PARENTID_CONST_STR",";

			if(parentTableDBIDs.size() > 1)
			{
				sqlstr1_ += " where " TABLE_PARENTID_CONST_STR " in(";
				std::vector<DBID>::const_iterator iter = parentTableDBIDs.begin();
				for(; iter != parentTableDBIDs.end(); ++iter)
				{
					kbe_snprintf(strdbid, MAX_BUF, "%" PRDBID ",", (*iter));
					sqlstr1_ += strdbid;
				}

				sqlstr1_.erase(sqlstr1_.end() - 1);
				sqlstr1_ += ")";
			}
			else
			{
				sqlstr1_ += " where " TABLE_PARENTID_CONST_STR "=";
				kbe_snprintf(strdbid, MAX_BUF, "%" PRDBID, parentTableDBIDs[0]);
				sqlstr1_ += strdbid;
			}
		}

		mysql::DBContext::DB_ITEM_DATAS::iterator tableValIter = tableItemDatas.begin();
		for(; tableValIter != tableItemDatas.end(); ++tableValIter)
		{
			KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA> pSotvs = (*tableValIter);
			
			sqlstr_ += pSotvs->sqlkey;
			sqlstr_ += ",";
		}

		if(sqlstr_.at(sqlstr_.size() - 1) == ',')
			sqlstr_.erase(sqlstr_.size() - 1);

		sqlstr_ += sqlstr1_;
	}

	virtual ~SqlStatementQuery()
	{
	}

protected:
	std::string sqlstr1_;
};

}
#endif // KBE_SQL_STATEMENT_H
