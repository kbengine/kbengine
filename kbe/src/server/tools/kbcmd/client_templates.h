/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#ifndef KBE_CLIENT_TEMPLATES_H
#define KBE_CLIENT_TEMPLATES_H

#include "common/common.h"
#include "helper/debug_helper.h"

namespace KBEngine{

class ScriptDefModule;
class PropertyDescription;
class MethodDescription;
class FixedDictType;
class FixedArrayType;
class DataType;

class ClientTemplates
{
public:
	ClientTemplates();
	virtual ~ClientTemplates();

	virtual bool good() const;

	virtual bool create(const std::string& path);

	virtual void onCreateModuleFileName(const std::string& moduleName);

	virtual bool writeTypes();
	virtual bool writeTypesBegin();
	virtual bool writeTypesEnd();
	virtual void onCreateTypeFileName();

	virtual bool writeTypeBegin(std::string typeName, FixedDictType* pDataType) { return false;  }
	virtual bool writeTypeEnd(std::string typeName, FixedDictType* pDataType) { return false; }

	virtual bool writeTypeBegin(std::string typeName, FixedArrayType* pDataType, const std::string& parentClass) { return false; }
	virtual bool writeTypeEnd(std::string typeName, FixedArrayType* pDataType) { return false; }

	virtual std::string typeToType(const std::string& type) {
		return "unknown";
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

	virtual bool writeTypeItemType_MAILBOX(const std::string& itemName, const std::string& childItemName)
	{
		return false;
	}



	virtual bool saveFile();

	virtual bool writeModule(ScriptDefModule* pEntityScriptDefModule);
	virtual bool writeModuleBegin(ScriptDefModule* pEntityScriptDefModule);
	virtual bool writeModuleEnd(ScriptDefModule* pEntityScriptDefModule);

	virtual bool writePropertys(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule);

	virtual bool writeProperty(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription);

	virtual bool writeProperty_INT8(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_INT16(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_INT32(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_INT64(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_UINT8(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_UINT16(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_UINT32(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_UINT64(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_FLOAT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_DOUBLE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_STRING(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_UNICODE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_PYTHON(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_PY_DICT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_PY_TUPLE(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_PY_LIST(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_BLOB(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_ARRAY(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_FIXED_DICT(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_VECTOR2(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_VECTOR3(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_VECTOR4(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeProperty_MAILBOX(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription) {
		return false;
	}

	virtual bool writeMethods(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule);

	virtual bool writeMethod(ScriptDefModule* pEntityScriptDefModule,
		ScriptDefModule* pCurrScriptDefModule, MethodDescription* pMethodDescription, const char* fillString);

	virtual bool writeMethodArgs_ARRAY(FixedArrayType* pFixedArrayType, std::string& stackArgsTypeBody, const std::string& childItemName);
	virtual bool writeMethodArgs_Const_Ref(DataType* pDataType, std::string& stackArgsTypeBody);

	static ClientTemplates* createClientTemplates(const std::string& type);

protected:
	std::string path_;
	std::string sourcefileBody_;
	std::string sourcefileName_;
};

}
#endif
