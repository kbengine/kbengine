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


namespace KBEngine { 


//-------------------------------------------------------------------------------------
INLINE void ScriptDefModule::setScriptType(PyTypeObject* scriptType)
{ 
	scriptType_ = scriptType;
}

//-------------------------------------------------------------------------------------
INLINE DetailLevel& ScriptDefModule::getDetailLevel(void)
{
	return detailLevel_; 
}

//-------------------------------------------------------------------------------------
INLINE VolatileInfo* ScriptDefModule::getPVolatileInfo(void)
{
	return pVolatileinfo_;
}

//-------------------------------------------------------------------------------------
INLINE void ScriptDefModule::setCell(bool have)
{ 
	hasCell_ = have; 
}

//-------------------------------------------------------------------------------------
INLINE void ScriptDefModule::setBase(bool have)
{ 
	hasBase_ = have; 
}

//-------------------------------------------------------------------------------------
INLINE void ScriptDefModule::setClient(bool have)
{ 
	hasClient_ = have; 
}

//-------------------------------------------------------------------------------------
INLINE bool ScriptDefModule::hasCell(void) const
{ 
	return hasCell_; 
}

//-------------------------------------------------------------------------------------
INLINE bool ScriptDefModule::hasBase(void) const
{ 
	return hasBase_; 
}

//-------------------------------------------------------------------------------------
INLINE bool ScriptDefModule::hasClient(void) const
{ 
	return hasClient_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_MAP& ScriptDefModule::getCellPropertyDescriptions()
{
	return cellPropertyDescr_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_MAP& ScriptDefModule::getCellPropertyDescriptionsByDetailLevel(int8 detailLevel)
{ 
	return cellDetailLevelPropertyDescrs_[detailLevel]; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_MAP& ScriptDefModule::getBasePropertyDescriptions()
{ 
	return basePropertyDescr_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_MAP& ScriptDefModule::getClientPropertyDescriptions()
{ 
	return clientPropertyDescr_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_MAP& ScriptDefModule::getPersistentPropertyDescriptions()
{
	return persistentPropertyDescr_;
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP& ScriptDefModule::getCellPropertyDescriptions_uidmap()
{ 
	return cellPropertyDescr_uidmap_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP& ScriptDefModule::getBasePropertyDescriptions_uidmap()
{
	return basePropertyDescr_uidmap_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP& ScriptDefModule::getClientPropertyDescriptions_uidmap()
{
	return clientPropertyDescr_uidmap_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::PROPERTYDESCRIPTION_UIDMAP& ScriptDefModule::getPersistentPropertyDescriptions_uidmap()
{
	return persistentPropertyDescr_uidmap_;
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::METHODDESCRIPTION_MAP& ScriptDefModule::getCellMethodDescriptions(void)
{ 
	return methodCellDescr_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::METHODDESCRIPTION_MAP& ScriptDefModule::getBaseMethodDescriptions(void)
{	
	return methodBaseDescr_; 
}

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::METHODDESCRIPTION_MAP& ScriptDefModule::getClientMethodDescriptions(void)
{ 
	return methodClientDescr_; 
}	

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::METHODDESCRIPTION_MAP& ScriptDefModule::getBaseExposedMethodDescriptions(void)
{ 
	return methodBaseExposedDescr_; 
}	

//-------------------------------------------------------------------------------------
INLINE ScriptDefModule::METHODDESCRIPTION_MAP& ScriptDefModule::getCellExposedMethodDescriptions(void)
{ 
	return methodCellExposedDescr_; 
}	

//-------------------------------------------------------------------------------------
INLINE const char* ScriptDefModule::getName()
{
	return name_.c_str(); 
}

//-------------------------------------------------------------------------------------
INLINE bool ScriptDefModule::isPersistent() const
{
	return persistentPropertyDescr_uidmap_.size() > 0;
}

//-------------------------------------------------------------------------------------
INLINE bool ScriptDefModule::usePropertyDescrAlias() const
{
	return usePropertyDescrAlias_;
}

//-------------------------------------------------------------------------------------
INLINE bool ScriptDefModule::useMethodDescrAlias() const
{
	return useMethodDescrAlias_;
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_SCRIPT_UID ScriptDefModule::getUType(void)
{
	return uType_;
}

//-------------------------------------------------------------------------------------
INLINE ENTITY_DEF_ALIASID ScriptDefModule::getAliasID(void)
{
	return (ENTITY_DEF_ALIASID)uType_;
}

//-------------------------------------------------------------------------------------
INLINE PyTypeObject* ScriptDefModule::getScriptType(void)
{
	return scriptType_;
}

//-------------------------------------------------------------------------------------
}
