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

#ifndef KBE_ENTITY_SQL_STATEMENT_MAPPING_H
#define KBE_ENTITY_SQL_STATEMENT_MAPPING_H

// common include	
// #define NDEBUG
#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"

namespace KBEngine{ 

class SqlStatement;
class EntitySqlStatementMapping : public Singleton<EntitySqlStatementMapping>
{
public:
	EntitySqlStatementMapping()
	{
	}

	virtual ~EntitySqlStatementMapping()
	{
	}

	void addQuerySqlStatement(const std::string& tablename, SqlStatement* pSqlStatement);
	void addInsertSqlStatement(const std::string& tablename, SqlStatement* pSqlStatement);
	void addUpdateSqlStatement(const std::string& tablename, SqlStatement* pSqlStatement);

	SqlStatement* findQuerySqlStatement(const std::string& tablename);
	SqlStatement* findInsertSqlStatement(const std::string& tablename);
	SqlStatement* findUpdateSqlStatement(const std::string& tablename);
protected:
	KBEUnordered_map< std::string, KBEShared_ptr< SqlStatement > > query_sqlStatements_;
	KBEUnordered_map< std::string, KBEShared_ptr< SqlStatement > > update_sqlStatements_;
	KBEUnordered_map< std::string, KBEShared_ptr< SqlStatement > > insert_sqlStatements_;
};



}
#endif
