// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
	return persistent_;
}

//-------------------------------------------------------------------------------------
INLINE void ScriptDefModule::isPersistent(bool v)
{
	persistent_ = v;
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
