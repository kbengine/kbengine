// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CELLS_H
#define KBE_CELLS_H

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

	ArraySize size() const{ return (ArraySize)cells_.size(); }

private:
	std::map<CELL_ID, Cell> cells_;
};

}
#endif
