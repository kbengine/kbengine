
// guiconsole.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int g_diffHeight = 0;
bool g_isDestroyed = false;

// CguiconsoleApp

BEGIN_MESSAGE_MAP(CguiconsoleApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

int CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort)
{
	ListSortData *p = (ListSortData *)lParamSort;
	CListCtrl* list = p->listctrl;
	int isub = p->isub;
	LVFINDINFO findInfo;
	findInfo.flags = LVFI_PARAM;
	findInfo.lParam = lParam1;
	int iItem1 = list->FindItem(&findInfo, -1);
	findInfo.lParam = lParam2;
	int iItem2 = list->FindItem(&findInfo, -1);

	CString strItem1 =list->GetItemText(iItem1,isub);
	CString strItem2 =list->GetItemText(iItem2,isub);
	if(p->seq)
		return _tcscmp(strItem2, strItem1);
	else
		return -_tcscmp(strItem2, strItem1);
}

int CALLBACK CompareFloatFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort)
{
	ListSortData *p = (ListSortData *)lParamSort;
	CListCtrl* list = p->listctrl;
	int isub = p->isub;
	LVFINDINFO findInfo;
	findInfo.flags = LVFI_PARAM;
	findInfo.lParam = lParam1;
	int iItem1 = list->FindItem(&findInfo, -1);
	findInfo.lParam = lParam2;
	int iItem2 = list->FindItem(&findInfo, -1);

	CString strItem1 =list->GetItemText(iItem1,isub);
	CString strItem2 =list->GetItemText(iItem2,isub);

	char* cs1 = KBEngine::strutil::wchar2char(strItem1.GetBuffer(0));
	char* cs2 = KBEngine::strutil::wchar2char(strItem2.GetBuffer(0));
	double v1 = atof(cs1);
	double v2 = atof(cs2);
	free(cs1);
	free(cs2);

	if(p->seq)
		return cs1 < cs2;
	else
		return cs1 > cs2;
}

// CguiconsoleApp construction

CguiconsoleApp::CguiconsoleApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CguiconsoleApp object

CguiconsoleApp theApp;


// CguiconsoleApp initialization

BOOL CguiconsoleApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	DebugHelper::initialize(CONSOLE_TYPE);

	CguiconsoleDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
