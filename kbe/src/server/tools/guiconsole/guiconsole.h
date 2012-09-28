
// guiconsole.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "cstdkbe/cstdkbe.hpp"
#include "network/endpoint.hpp"
#include "network/common.hpp"
#include "network/channel.hpp"
#include "network/tcp_packet.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "helper/debug_helper.hpp"
#include "xmlplus/xmlplus.hpp"
#include "cstdkbe/smartpointer.hpp"

// CguiconsoleApp:
// See guiconsole.cpp for the implementation of this class
//

class CguiconsoleApp : public CWinAppEx
{
public:
	CguiconsoleApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CguiconsoleApp theApp;