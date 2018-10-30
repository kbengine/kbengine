// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_FIXED_NETWORK_MESSAGES_H
#define KBE_FIXED_NETWORK_MESSAGES_H

#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "network/common.h"

namespace KBEngine { 
namespace Network
{
/*
	用来对中间协议(前端与后端之间)进行强制约定。
	没有使用到kbe整套协议自动绑定机制的前端可以使用此处来强制约定协议。
*/
class FixedMessages : public Singleton<FixedMessages>
{
public:

	// 固定的协议数据结构
	struct MSGInfo
	{
		MessageID msgid;
		std::string msgname;
		//std::wstring descr;
	};

public:
	FixedMessages();
	~FixedMessages();

	bool loadConfig(std::string fileName, bool notFoundError = true);

	FixedMessages::MSGInfo* isFixed(const char* msgName);
	bool isFixed(MessageID msgid);

public:
	typedef KBEUnordered_map<std::string, MSGInfo> MSGINFO_MAP;

private:
	MSGINFO_MAP _infomap;
	bool _loaded;
};

}
}
#endif // KBE_FIXED_NETWORK_MESSAGES_H
