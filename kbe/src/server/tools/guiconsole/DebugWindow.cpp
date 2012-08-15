// DebugWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "DebugWindow.h"


// CDebugWindow dialog

IMPLEMENT_DYNAMIC(CDebugWindow, CDialog)

CDebugWindow::CDebugWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugWindow::IDD, pParent)
{

}

CDebugWindow::~CDebugWindow()
{
}

void CDebugWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_displaybuffer);
	DDX_Control(pDX, IDC_EDIT2, m_sendbuffer);
}


BEGIN_MESSAGE_MAP(CDebugWindow, CDialog)
	ON_WM_SIZING()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CDebugWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE; 
};

// CDebugWindow message handlers

void CDebugWindow::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	
	// TODO: Add your message handler code here
}

void CDebugWindow::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

void CDebugWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
	m_displaybuffer.MoveWindow(5, 5, rect.right, (int)(rect.bottom * 0.75), TRUE);
	m_sendbuffer.MoveWindow(5, int(rect.bottom * 0.76), rect.right, int(rect.bottom * 0.25), TRUE);
}
