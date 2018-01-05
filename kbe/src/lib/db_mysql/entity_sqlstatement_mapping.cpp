/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#include "entity_sqlstatement_mapping.h"
#include "sqlstatement.h"

namespace KBEngine{ 
KBE_SINGLETON_INIT(EntitySqlStatementMapping);

EntitySqlStatementMapping _g_entitySqlStatementMapping;

//-------------------------------------------------------------------------------------
void EntitySqlStatementMapping::addQuerySqlStatement(const std::string& tableName, SqlStatement* pSqlStatement)
{
	query_sqlStatements_[tableName].reset(pSqlStatement);
}

//-------------------------------------------------------------------------------------
void EntitySqlStatementMapping::addInsertSqlStatement(const std::string& tableName, SqlStatement* pSqlStatement)
{
	insert_sqlStatements_[tableName].reset(pSqlStatement);
}

//-------------------------------------------------------------------------------------
void EntitySqlStatementMapping::addUpdateSqlStatement(const std::string& tableName, SqlStatement* pSqlStatement)
{
	update_sqlStatements_[tableName].reset(pSqlStatement);
}

//-------------------------------------------------------------------------------------
SqlStatement* EntitySqlStatementMapping::findQuerySqlStatement(const std::string& tableName)
{
	KBEUnordered_map< std::string, KBEShared_ptr< SqlStatement > >::iterator iter = 
		query_sqlStatements_.find(tableName);

	if(iter != query_sqlStatements_.end())
		return iter->second.get();

	return NULL;
}

//-------------------------------------------------------------------------------------
SqlStatement* EntitySqlStatementMapping::findInsertSqlStatement(const std::string& tableName)
{
	KBEUnordered_map< std::string, KBEShared_ptr< SqlStatement > >::iterator iter = 
		insert_sqlStatements_.find(tableName);

	if(iter != insert_sqlStatements_.end())
		return iter->second.get();

	return NULL;
}

//-------------------------------------------------------------------------------------
SqlStatement* EntitySqlStatementMapping::findUpdateSqlStatement(const std::string& tableName)
{
	KBEUnordered_map< std::string, KBEShared_ptr< SqlStatement > >::iterator iter = 
		update_sqlStatements_.find(tableName);

	if(iter != update_sqlStatements_.end())
		return iter->second.get();

	return NULL;
}

//-------------------------------------------------------------------------------------
}

