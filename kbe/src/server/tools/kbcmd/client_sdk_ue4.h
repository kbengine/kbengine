// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CLIENT_SDK_UE4_H
#define KBE_CLIENT_SDK_UE4_H

#include "client_sdk.h"
#include "common/common.h"
#include "helper/debug_helper.h"

namespace KBEngine{

class ClientSDKUE4 : public ClientSDK
{
public:
	ClientSDKUE4();
	virtual ~ClientSDKUE4();

	virtual std::string name() const {
		return "ue4";
	}

	virtual void onCreateEntityModuleFileName(const std::string& moduleName);
	virtual void onCreateTypeFileName();
	virtual void onCreateServerErrorDescrsModuleFileName();
	virtual void onCreateEngineMessagesModuleFileName();
	virtual void onCreateEntityDefsModuleFileName();
	virtual void onCreateDefsCustomTypesModuleFileName();
	virtual void onEntityCallModuleFileName(const std::string& moduleName);

	virtual bool writeServerErrorDescrsModuleBegin();
	virtual bool writeServerErrorDescrsModuleErrDescr(int errorID, const std::string& errname, const std::string& errdescr);
	virtual bool writeServerErrorDescrsModuleEnd();

	
	virtual bool writeEngineMessagesModuleBegin();
	virtual bool writeEngineMessagesModuleMessage(Network::ExposedMessageInfo& messageInfos, COMPONENT_TYPE componentType);
	virtual bool writeEngineMessagesModuleEnd();

	virtual bool writeEntityDefsModuleBegin();
	virtual bool writeEntityDefsModuleEnd();

	virtual bool writeEntityDefsModuleInitScript_ScriptModule(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityDefsModuleInitScript_MethodDescr(ScriptDefModule* pScriptDefModule, MethodDescription* pDescr, COMPONENT_TYPE componentType);
	virtual bool writeEntityDefsModuleInitScript_PropertyDescr(ScriptDefModule* pScriptDefModule, PropertyDescription* pDescr);

	virtual bool writeEntityDefsModuleInitDefType(const DataType* pDataType);
	virtual bool writeEntityDefsModuleInitDefTypesBegin();
	virtual bool writeEntityDefsModuleInitDefTypesEnd();

	virtual bool writeCustomDataTypesBegin();
	virtual bool writeCustomDataTypesEnd();
	virtual bool writeCustomDataType(const DataType* pDataType);

	virtual bool writeEntityCallBegin(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityCallEnd(ScriptDefModule* pScriptDefModule);
	virtual bool writeBaseEntityCallBegin(ScriptDefModule* pScriptDefModule);
	virtual bool writeBaseEntityCallEnd(ScriptDefModule* pScriptDefModule);
	virtual bool writeCellEntityCallBegin(ScriptDefModule* pScriptDefModule);
	virtual bool writeCellEntityCallEnd(ScriptDefModule* pScriptDefModule);
	virtual bool writeEntityCallMethodBegin(ScriptDefModule* pScriptDefModule, MethodDescription* pMethodDescription, 
		const char* fillString1, const char* fillString2, COMPONENT_TYPE componentType);
	virtual bool writeEntityCallMethodEnd(ScriptDefModule* pScriptDefModule, MethodDescription* pMethodDescription);

	virtual std::string typeToType(const std::string& type);
	virtual bool getArrayType(DataType* pDataType, std::string& outstr);
	virtual bool createArrayChildClass(DataType* pRootDataType, DataType* pDataType, const std::string& className, const std::string& tabs, int numLayer = 1);

	virtual bool writeTypesBegin();
	virtual bool writeTypesEnd();

	virtual bool writeTypeBegin(std::string typeName, FixedDictType* pDataType);
	virtual bool writeTypeEnd(std::string typeName, FixedDictType* pDataType);

	virtual bool writeTypeBegin(std::string typeName, DataType* pDataType);
	virtual bool writeTypeEnd(std::string typeName, DataType* pDataType);

	virtual bool writeTypeBegin(std::string typeName, FixedArrayType* pDataType, const std::string& parentClass);
	virtual bool writeTypeEnd(std::string typeName, FixedArrayType* pDataType);

	virtual bool writeTypeItemType_AliasName(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_INT8(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_INT16(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_INT32(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_INT64(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_UINT8(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_UINT16(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_UINT32(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_UINT64(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_FLOAT(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_DOUBLE(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_STRING(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_UNICODE(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_PYTHON(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_PY_DICT(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_PY_TUPLE(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_PY_LIST(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_BLOB(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_ARRAY(const std::string& itemName, const std::string& childItemName, DataType* pDataType);
	virtual bool writeTypeItemType_FIXED_DICT(const std::string& itemName, const std::string& childItemName, DataType* pDataType);
	virtual bool writeTypeItemType_VECTOR2(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_VECTOR3(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_VECTOR4(const std::string& itemName, const std::string& childItemName);
	virtual bool writeTypeItemType_ENTITYCALL(const std::string& itemName, const std::string& childItemName);

	virtual bool writeEntityModuleBegin(ScriptDefModule* pEntityScriptDefModule);
	virtual bool writeEntityModuleEnd(ScriptDefModule* pEntityScriptDefModule);

	virtual bool writeEntityDefsModuleInitScriptBegin();
	virtual bool writeEntityDefsModuleInitScriptEnd();

	virtual bool writeEntityProcessMessagesMethod(ScriptDefModule* pEntityScriptDefModule);

	virtual bool writeEntityPropertyComponent(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_INT8(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_INT16(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_INT32(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_INT64(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_UINT8(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_UINT16(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_UINT32(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_UINT64(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_FLOAT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_DOUBLE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_STRING(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_UNICODE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_PYTHON(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_PY_DICT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_PY_TUPLE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_PY_LIST(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_BLOB(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_ARRAY(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_FIXED_DICT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_VECTOR2(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_VECTOR3(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_VECTOR4(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityProperty_ENTITYCALL(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeEntityMethod(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, MethodDescription* pMethodDescription, const char* fillString);

	virtual bool writeEntityMethodArgs_ARRAY(FixedArrayType* pFixedArrayType, std::string& stackArgsTypeBody, const std::string& childItemName);
	virtual bool writeEntityMethodArgs_Const_Ref(DataType* pDataType, std::string& stackArgsTypeBody);

	void changeContextToHeader() {
		contextHeader_ = true;
	}

	void changeContextToSource() {
		contextHeader_ = false;
	}

	bool isContextToHeader() {
		return contextHeader_;
	}

	std::string& fileBody() {
		return isContextToHeader() ? headerfileBody_ : sourcefileBody_;
	}

protected:
	std::string initBody_;
	bool contextHeader_;
};

}
#endif
