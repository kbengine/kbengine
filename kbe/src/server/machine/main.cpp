/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "cstdkbe/cstdkbe.hpp"
#include "server/kbemain.hpp"
#include "machine.hpp"

using namespace KBEngine;

int KBENGINE_MAIN(int argc, char* argv[])
{

#if KBE_PLATFORM != PLATFORM_WIN32
	rlimit rlimitData = { RLIM_INFINITY, RLIM_INFINITY };
	setrlimit(RLIMIT_CORE, &rlimitData);
#endif

	int ret = kbeMainT<Machine>(argc, argv, MACHINE_TYPE, KBE_MACHINE_TCP_PORT);
	return ret; 
}
