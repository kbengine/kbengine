/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "server/kbemain.hpp"
#include "cellapp.hpp"

using namespace KBEngine;

int KBENGINE_MAIN(int argc, char* argv[])
{
	int ret= kbeMainT<CellApp>(argc, argv, CELLAPP_TYPE);
	getchar();
	return ret; 
}
