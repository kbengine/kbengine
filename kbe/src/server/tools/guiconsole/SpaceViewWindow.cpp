// SpaceViewWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "SpaceViewWindow.h"


// CSpaceViewWindow dialog

IMPLEMENT_DYNAMIC(CSpaceViewWindow, CDialog)

CSpaceViewWindow::CSpaceViewWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CSpaceViewWindow::IDD, pParent)
{

}

CSpaceViewWindow::~CSpaceViewWindow()
{
}

void CSpaceViewWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CSpaceViewWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
}

BEGIN_MESSAGE_MAP(CSpaceViewWindow, CDialog)
END_MESSAGE_MAP()


// CSpaceViewWindow message handlers
