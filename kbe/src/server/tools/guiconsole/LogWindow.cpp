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
	DDX_Control(pDX, IDC_BUTTON1, m_autopull);
	DDX_Control(pDX, IDC_LOG_EDIT1, m_logedit);
}


BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
END_MESSAGE_MAP()

void CLogWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);

	m_autopull.MoveWindow(int(rect.right * 0.33) + 3, int(rect.bottom * 0.95), rect.right / 3, int(rect.bottom * 0.05), TRUE);
	m_logedit.MoveWindow(2, 3, rect.right, rect.bottom - int(rect.bottom * 0.05) - 5, TRUE);
}

// CLogWindow message handlers
