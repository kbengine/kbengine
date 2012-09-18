// ProfileWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "ProfileWindow.h"


// CProfileWindow dialog

IMPLEMENT_DYNAMIC(CProfileWindow, CDialog)

CProfileWindow::CProfileWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CProfileWindow::IDD, pParent)
{

}

CProfileWindow::~CProfileWindow()
{
}

void CProfileWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_pyprofile);
	DDX_Control(pDX, IDC_BUTTON2, m_cprofile);
	DDX_Control(pDX, IDC_BUTTON3, m_eventprofile);
	DDX_Control(pDX, IDC_LIST1, m_profileShowList);
}

void CProfileWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);
	m_pyprofile.MoveWindow(2, int(rect.bottom * 0.95), rect.right / 3, int(rect.bottom * 0.05), TRUE);
	m_cprofile.MoveWindow(int(rect.right * 0.33) + 3, int(rect.bottom * 0.95), rect.right / 3, int(rect.bottom * 0.05), TRUE);
	m_eventprofile.MoveWindow(int(rect.right * 0.66) + 3, int(rect.bottom * 0.95), rect.right / 3, int(rect.bottom * 0.05), TRUE);
	
	m_profileShowList.MoveWindow(2, 3, rect.right, rect.bottom - int(rect.bottom * 0.05) - 5, TRUE);
}


BEGIN_MESSAGE_MAP(CProfileWindow, CDialog)
END_MESSAGE_MAP()


// CProfileWindow message handlers
