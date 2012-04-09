/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "cstdkbe/platform.hpp"
#ifndef __KBEVERSION_HPP__
#define __KBEVERSION_HPP__
namespace KBEngine{
	
#define KBE_VERSION_MAJOR 0
#define KBE_VERSION_MINOR 0
#define KBE_VERSION_PATCH 1

namespace KBEVersion
{
	const std::string & versionString();
}

}
#endif // __KBEVERSION_HPP__
