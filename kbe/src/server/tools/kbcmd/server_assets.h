// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SERVER_ASSETS_H
#define KBE_SERVER_ASSETS_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "network/message_handler.h"

namespace KBEngine{

class ServerAssets
{
public:
	ServerAssets();
	virtual ~ServerAssets();

	virtual bool good() const;

	virtual bool create(const std::string& path);

	static ServerAssets* createServerAssets(const std::string& type);

	virtual std::string name() const {
		return "python_assets";
	}

	virtual bool copyAssetsSourceToPath(const std::string& path);

protected:
	std::string basepath_;

};

}
#endif
