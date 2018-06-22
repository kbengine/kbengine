// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "updatable.h"	
namespace KBEngine{	


//-------------------------------------------------------------------------------------
Updatable::Updatable():
removeIdx(-1),
updatableName("Updatable")
{
}

//-------------------------------------------------------------------------------------
Updatable::~Updatable()
{
	removeIdx = -1;
}

//-------------------------------------------------------------------------------------
}
