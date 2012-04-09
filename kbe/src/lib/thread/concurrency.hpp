/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef __CONCURENCY_HPP__
#define __CONCURENCY_HPP__

#include "cstdkbe/platform.hpp"
#include "helper/debug_helper.hpp"
namespace KBEngine{

extern void (*pMainThreadIdleStartFunc)();
extern void (*pMainThreadIdleEndFunc)();

namespace KBEConcurrency
{
inline void startMainThreadIdling()
{
	(*pMainThreadIdleStartFunc)();
}

inline void endMainThreadIdling()
{
	(*pMainThreadIdleEndFunc)();
}

inline
void setMainThreadIdleFunctions( void (*pStartFunc)(), void (*pEndFunc)() )
{
	pMainThreadIdleStartFunc = pStartFunc;
	pMainThreadIdleEndFunc = pEndFunc;
}

}

}

#endif // __TIMESTAMP_HPP__
