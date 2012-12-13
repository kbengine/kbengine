// GraphsWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "GraphsWindow.h"


// CGraphsWindow dialog

IMPLEMENT_DYNAMIC(CGraphsWindow, CDialog)

CGraphsWindow::CGraphsWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CGraphsWindow::IDD, pParent)
{

}

CGraphsWindow::~CGraphsWindow()
{
}

void CGraphsWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGraphsWindow, CDialog)
END_MESSAGE_MAP()


BOOL CGraphsWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE; 
};

void CGraphsWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
}

// CGraphsWindow message handlers
