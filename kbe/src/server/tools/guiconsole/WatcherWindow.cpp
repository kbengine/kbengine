// WatcherWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "WatcherWindow.h"


// CWatcherWindow dialog

IMPLEMENT_DYNAMIC(CWatcherWindow, CDialog)

CWatcherWindow::CWatcherWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CWatcherWindow::IDD, pParent)
{

}

CWatcherWindow::~CWatcherWindow()
{
}

void CWatcherWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CWatcherWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
}

BEGIN_MESSAGE_MAP(CWatcherWindow, CDialog)
END_MESSAGE_MAP()


// CWatcherWindow message handlers
