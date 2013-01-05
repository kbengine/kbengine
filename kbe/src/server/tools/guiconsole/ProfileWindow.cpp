// ProfileWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "ProfileWindow.h"
#include "TimingLengthWindow.h"

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

BOOL CProfileWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	DWORD dwStyle = m_profileShowList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_ONECLICKACTIVATE;
	m_profileShowList.SetExtendedStyle(dwStyle);				//设置扩展风格

	int idx = 0;
	m_profileShowList.InsertColumn(idx++, _T("ncalls "),					LVCFMT_CENTER,	50);
	m_profileShowList.InsertColumn(idx++, _T("tottime"),					LVCFMT_CENTER,	80);
	m_profileShowList.InsertColumn(idx++, _T("percall"),					LVCFMT_CENTER,	80);
	m_profileShowList.InsertColumn(idx++, _T("cumtime"),					LVCFMT_CENTER,	80);
	m_profileShowList.InsertColumn(idx++, _T("percall"),					LVCFMT_CENTER,	80);
	m_profileShowList.InsertColumn(idx++, _T("filename:lineno(function)"),	LVCFMT_CENTER,	300);
	return TRUE;  // return TRUE  unless you set the focus to a control
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
	ON_BN_CLICKED(IDC_BUTTON1, &CProfileWindow::OnBnClickedButton1)
END_MESSAGE_MAP()


// CProfileWindow message handlers

void CProfileWindow::OnBnClickedButton1()
{
	m_pyprofile.EnableWindow(FALSE);

	CTimingLengthWindow wnd;
	if(wnd.DoModal() == IDOK)
	{
		return;
	}

	m_pyprofile.EnableWindow(TRUE);
}
