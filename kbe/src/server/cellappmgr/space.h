// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_MGR_SPACE_H
#define KBE_MGR_SPACE_H

// common include
#include "cells.h"
#include "helper/debug_helper.h"
#include "common/common.h"


namespace KBEngine{

class Space
{
public:
	Space();
	~Space();

	void updateGeomappingPath(const std::string& geomappingPath);
	void updateScriptModuleName(const std::string& scriptModuleName);
	void setSpaceID(SPACE_ID spaceID) { spaceID_ = spaceID; }

	SPACE_ID id() const { return spaceID_; }
	std::string& getGeomappingPath() { return geomappingPath_; }
	std::string& getScriptModuleName() { return scriptModuleName_; }

	Cells& cells() { return cells_; }

private:
	SPACE_ID spaceID_;
	Cells cells_;

	std::string geomappingPath_;
	std::string scriptModuleName_;
};

}
#endif
