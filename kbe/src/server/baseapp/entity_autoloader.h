// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ENTITY_AUTOLOADER_H
#define KBE_ENTITY_AUTOLOADER_H

#include "common/common.h"

namespace KBEngine{

class InitProgressHandler;
class EntityAutoLoader
{
public:
	EntityAutoLoader(Network::NetworkInterface & networkInterface, InitProgressHandler* pInitProgressHandler);
	~EntityAutoLoader();
	
	bool process();

	void pInitProgressHandler(InitProgressHandler* p)
		{ pInitProgressHandler_ = p; }

	/** 网络接口
		数据库中查询的自动entity加载信息返回
	*/
	void onEntityAutoLoadCBFromDBMgr(Network::Channel* pChannel, MemoryStream& s);

private:
	Network::NetworkInterface & networkInterface_;
	InitProgressHandler* pInitProgressHandler_;

	std::vector< std::vector<ENTITY_SCRIPT_UID> > entityTypes_;

	// 每次取查询结果集的区段
	ENTITY_ID start_;
	ENTITY_ID end_;

	bool querying_;
};


}

#endif // KBE_ENTITY_AUTOLOADER_H
