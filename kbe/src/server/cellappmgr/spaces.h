// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_MGR_SPACES_H
#define KBE_MGR_SPACES_H

#include "space.h"
#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{


class Spaces
{
public:
	Spaces();
	~Spaces();

	size_t size() const {
		return spaces_.size();
	}

	void updateSpaceData(SPACE_ID spaceID, const std::string& scriptModuleName, const std::string& geomappingPath, bool deleted);

	Space* getSpace(SPACE_ID id) {
		std::map<SPACE_ID, Space>::iterator iter = spaces_.find(id);
		if (iter == spaces_.end())
			return NULL;

		return &iter->second;
	}

	std::map<SPACE_ID, Space>& spaces() {
		return spaces_;
	}

private:
	std::map<SPACE_ID, Space> spaces_;
};

}
#endif
