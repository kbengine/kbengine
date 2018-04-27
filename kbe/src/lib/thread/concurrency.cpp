// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "concurrency.h"

namespace KBEngine
{

static void default_op() {}

void (*pMainThreadIdleStartCallback)() = &default_op;
void (*pMainThreadIdleEndCallback)() = &default_op;

}
