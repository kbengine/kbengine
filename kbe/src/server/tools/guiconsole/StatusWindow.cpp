// StatusWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "StatusWindow.h"


// StatusWindow dialog

IMPLEMENT_DYNAMIC(StatusWindow, CDialog)

StatusWindow::StatusWindow(CWnd* pParent /*=NULL*/)
	: CDialog(StatusWindow::IDD, pParent)
{

}

StatusWindow::~StatusWindow()
{
}

void StatusWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(StatusWindow, CDialog)
END_MESSAGE_MAP()

void StatusWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
}
// StatusWindow message handlers
