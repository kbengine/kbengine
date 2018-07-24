// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CLIENT_SDK_H
#define KBE_CLIENT_SDK_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "network/message_handler.h"

namespace KBEngine{

class ScriptDefModule;
class PropertyDescription;
class MethodDescription;
class FixedDictType;
class FixedArrayType;
class DataType;

class ClientSDK
{
public:
	ClientSDK();
	virtual ~ClientSDK();

	virtual bool good() const;

	virtual std::string name() const {
		return "unknown";
	}

	virtual bool create(const std::string& path);

	virtual void onCreateEntityModuleFileName(const std::string& moduleName);
	virtual void onCreateServerErrorDescrsModuleFileName();
	virtual void onCreateEngineMessagesModuleFileName();
	virtual void onCreateEntityDefsModuleFileName();
	virtual void onCreateDefsCustomTypesModuleFileName();
	virtual void onEntityCallModuleFileName(const std::string& moduleName);

	virtual bool copyPluginsSourceToPath(const std::string& path);

	virtual bool writeServerErrorDescrsModule();
	virtual bool writeServerErrorDescrsModuleBegin();
	virtual bool writeServerErrorDescrsModuleErrDescr(int errorID, const std::string& errname, const std::string& errdescr);
	virtual bool writeServerErrorDescrsModuleEnd();

	virtual bool writeEngineMessagesModule();
	virtual bool writeEngineMessagesModuleBegin();
	virtual bool writeEngineMessagesModuleMessage(Network::ExposedMessageInfo& messageInfos, COMPONENT_TYPE componentType);
	virtual bool writeEngineMessagesModuleEnd();

	bool writeEntityDefsModule();
	virtual bool writeEntityDefsModuleBegin();
	virtual bool writeEntityDefsModuleEnd();

	virtual bool writeEntityDefsModuleInitScriptBegin();
	virtual bool writeEntityDefsModuleInitScriptEnd();
	bool writeEntityDefsModuleInitScript(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityDefsModuleInitScript_ScriptModule(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityDefsModuleInitScript_MethodDescr(ScriptDefModule* pScriptDefModule, MethodDescription* pDescr, COMPONENT_TYPE componentType);
	virtual bool writeEntityDefsModuleInitScript_PropertyDescr(ScriptDefModule* pScriptDefModule, PropertyDescription* pDescr);

	bool writeEntityCall(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityCallBegin(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityCallEnd(ScriptDefModule* pScriptDefModule);
	virtual bool writeBaseEntityCallBegin(ScriptDefModule* pScriptDefModule);
	virtual bool writeBaseEntityCallEnd(ScriptDefModule* pScriptDefModule);
	virtual bool writeCellEntityCallBegin(ScriptDefModule* pScriptDefModule);
	virtual bool writeCellEntityCallEnd(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityCallMethodBegin(ScriptDefModule* pScriptDefModule, 
		MethodDescription* pMethodDescription, const char* fillString1, const char* fillString2, COMPONENT_TYPE componentType);
	virtual bool writeEntityCallMethodEnd(ScriptDefModule* pScriptDefModule, MethodDescription* pMethodDescription);

	virtual bool writeEntityDefsModuleInitDefTypesBegin();
	virtual bool writeEntityDefsModuleInitDefTypesEnd();
	bool writeEntityDefsModuleInitDefTypes();
	virtual bool writeEntityDefsModuleInitDefType(const DataType* pDataType);

	virtual bool writeCustomDataTypesBegin();
	virtual bool writeCustomDataTypesEnd();
	bool writeCustomDataTypes();
	virtual bool writeCustomDataType(const DataType* pDataType);

	virtual bool writeTypes();
	virtual bool writeTypesBegin();
	virtual bool writeTypesEnd();
	virtual void onCreateTypeFileName();

	virtual bool writeTypeBegin(std::string typeName, FixedDictType* pDataType) { return false;  }
	virtual bool writeTypeEnd(std::string typeName, FixedDictType* pDataType) { return false; }

	virtual bool writeTypeBegin(std::string typeName, DataType* pDataType) { return false; }
	virtual bool writeTypeEnd(std::string typeName, DataType* pDataType) { return false; }

	virtual bool writeTypeBegin(std::string typeName, FixedArrayType* pDataType, const std::string& parentClass) { return false; }
	virtual bool writeTypeEnd(std::string typeName, FixedArrayType* pDataType) { return false; }

	virtual std::string typeToType(const std::string& type) 
	{
		return "unknown";
	}

	virtual bool getArrayType(DataType* pDataType, std::string& outstr) 
	{
		return false;
	}

	virtual bool createArrayChildClass(DataType* pRootDataType, DataType* pDataType, const std::string& className, const std::string& tabs, int numLayer = 1)
	{
		return false;
	}

	virtual bool writeTypeItemType_INT8(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_INT16(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_INT32(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_INT64(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_UINT8(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_UINT16(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_UINT32(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_UINT64(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_FLOAT(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_DOUBLE(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_STRING(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_UNICODE(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_PYTHON(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_PY_DICT(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_PY_TUPLE(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_PY_LIST(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_BLOB(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_ARRAY(const std::string& itemName, const std::string& childItemName, DataType* pDataType)
	{
		return false;
	}

	virtual bool writeTypeItemType_FIXED_DICT(const std::string& itemName, const std::string& childItemName, DataType* pDataType)
	{
		return false;
	}

	virtual bool writeTypeItemType_VECTOR2(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_VECTOR3(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_VECTOR4(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_ENTITYCALL(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool writeTypeItemType_AliasName(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}

	virtual bool saveFile();

	virtual bool writeEntityModule(ScriptDefModule* pEntityScriptDefModule);
	virtual bool writeEntityModuleBegin(ScriptDefModule* pEntityScriptDefModule);
	virtual bool writeEntityModuleEnd(ScriptDefModule* pEntityScriptDefModule);

	virtual bool writeEntityPropertys(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule);

	virtual bool writeEntityPropertyComponent(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_INT8(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_INT16(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_INT32(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_INT64(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_UINT8(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_UINT16(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_UINT32(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_UINT64(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_FLOAT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_DOUBLE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_STRING(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_UNICODE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_PYTHON(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_PY_DICT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_PY_TUPLE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_PY_LIST(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_BLOB(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_ARRAY(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_FIXED_DICT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_VECTOR2(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_VECTOR3(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_VECTOR4(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityProperty_ENTITYCALL(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeEntityMethods(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule);

	virtual bool writeEntityMethod(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, MethodDescription* pMethodDescription, const char* fillString);

	virtual bool writeEntityMethodArgs_ARRAY(FixedArrayType* pFixedArrayType, std::string& stackArgsTypeBody, const std::string& childItemName);
	virtual bool writeEntityMethodArgs_Const_Ref(DataType* pDataType, std::string& stackArgsTypeBody);

	virtual bool writeEntityProcessMessagesMethod(ScriptDefModule* pEntityScriptDefModule);

	static ClientSDK* createClientSDK(const std::string& type);

protected:
	std::string basepath_, currSourcePath_, currHeaderPath_;
	std::string sourcefileBody_;
	std::string sourcefileName_;
	std::string headerfileName_;
	std::string headerfileBody_;
};

}
#endif
