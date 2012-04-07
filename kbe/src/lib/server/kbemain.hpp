/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBEMAIN__
#define __KBEMAIN__
#include "serverapp.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
namespace KBEngine{

template <class SERVER_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType)
{
	SERVER_APP app;
	if(!app.initialize(componentType)){
		ERROR_MSG("app::initialize is error!");
		return -1;
	}

	int ret = app.run();
	app.finalise();
	return ret;
}

#define KBENGINE_MAIN										\
kbeMain(int argc, char* argv[]);							\
int main(int argc, char* argv[])							\
{															\
	return kbeMain(argc, argv);								\
}															\
int kbeMain

}

#endif // __KBEMAIN__