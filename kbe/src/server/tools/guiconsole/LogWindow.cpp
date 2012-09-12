// LogWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "LogWindow.h"


// CLogWindow dialog

IMPLEMENT_DYNAMIC(CLogWindow, CDialog)

CLogWindow::CLogWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CLogWindow::IDD, pParent)
{

}

CLogWindow::~CLogWindow()
{
}

void CLogWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
END_MESSAGE_MAP()

void CLogWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
}

// CLogWindow message handlers
