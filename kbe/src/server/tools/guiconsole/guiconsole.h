
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

inline CString GetAppPath()
{
	::TCHAR modulePath[MAX_PATH];
    GetModuleFileName(NULL, modulePath, MAX_PATH);
    CString strModulePath(modulePath);
    strModulePath = strModulePath.Left(strModulePath.ReverseFind(_T('\\')));
    return strModulePath;
}

struct ListSortData{
     CListCtrl *listctrl;               //CListCtrl控件指针
     int isub;							//l列号
     int seq;							//1为升序，0为降序
};

int CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);

extern int g_diffHeight;