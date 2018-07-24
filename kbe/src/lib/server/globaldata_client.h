// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#ifndef KBE_GLOBAL_DATA_CLIENT_H
#define KBE_GLOBAL_DATA_CLIENT_H

#include "globaldata_server.h"
#include "common/common.h"
#include "helper/debug_helper.h"
#include "pyscript/map.h"

namespace KBEngine{

class GlobalDataClient : public script::Map
{	
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(GlobalDataClient, script::Map)
		
public:	
	GlobalDataClient(COMPONENT_TYPE componentType, GlobalDataServer::DATA_TYPE dataType);
	~GlobalDataClient();
	
	/** 写数据 */
	bool write(PyObject* pyKey, PyObject* pyValue);
	
	/** 删除数据 */
	bool del(PyObject* pyKey);
	
	/** 数据改变通知 */
	void onDataChanged(PyObject* key, PyObject* value, bool isDelete = false);
	
	/** 设置该全局数据客户端的服务器组件类型 */
	void setServerComponentType(COMPONENT_TYPE ct){ serverComponentType_ = ct; }
	
private:
	COMPONENT_TYPE					serverComponentType_;				// GlobalDataServer所在服务器组件的类型
	GlobalDataServer::DATA_TYPE 	dataType_;
} ;

}

#endif // KBE_GLOBAL_DATA_CLIENT_H
