// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "baseapp.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
Baseapp::Baseapp():
numEntitys_(0),
numProxices_(0),
load_(0.f),
isDestroyed_(false),
watchers_(),
initProgress_(0.f),
flags_(APP_FLAGS_NONE)
{
}

//-------------------------------------------------------------------------------------
Baseapp::~Baseapp()
{
}


//-------------------------------------------------------------------------------------
}
