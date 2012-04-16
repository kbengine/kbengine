/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "server/kbemain.hpp"

using namespace KBEngine;

class App: public ServerApp
{
protected:
public:
	App(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
	  ServerApp(dispatcher, ninterface, componentType)
	{
		
	}

	~App()
	{
	}


	bool run()
	{
		return ServerApp::run();
	}
};

template<> App* Singleton<App>::m_singleton_ = 0;

int KBENGINE_MAIN(int argc, char* argv[])
{
	int ret= kbeMainT<App>(argc, argv, CLIENT_TYPE);
	getchar();
	return ret; 
}
