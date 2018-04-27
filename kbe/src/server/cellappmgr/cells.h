// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_MGR_CELLS_H
#define KBE_MGR_CELLS_H

// common include
#include "cell.h"
#include "helper/debug_helper.h"
#include "common/common.h"


namespace KBEngine{

class Cells
{
public:
	Cells();
	~Cells();

	std::map<CELL_ID, Cell>& cells() {
		return cells_;
	}

private:
	std::map<CELL_ID, Cell> cells_;
};

}
#endif
