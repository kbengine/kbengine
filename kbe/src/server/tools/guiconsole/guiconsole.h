
// guiconsole.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "common/common.h"
#include "network/endpoint.h"
#include "network/common.h"
#include "network/channel.h"
#include "network/tcp_packet.h"
#include "network/interfaces.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "helper/debug_helper.h"
#include "xml/xml.h"
#include "common/smartpointer.h"

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
int CALLBACK CompareFloatFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);

extern int g_diffHeight;
extern bool g_isDestroyed;