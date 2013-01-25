// StartServerWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "StartServerWindow.h"
#include "StartServerLayoutWindow.h"

// CStartServerWindow dialog

IMPLEMENT_DYNAMIC(CStartServerWindow, CDialog)

CStartServerWindow::CStartServerWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CStartServerWindow::IDD, pParent)
{

}

CStartServerWindow::~CStartServerWindow()
{
}

void CStartServerWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CStartServerWindow, CDialog)
	ON_BN_CLICKED(IDC_BUTTON4, &CStartServerWindow::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON2, &CStartServerWindow::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CStartServerWindow::OnBnClickedButton3)
END_MESSAGE_MAP()


// CStartServerWindow message handlers
BOOL CStartServerWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_ONECLICKACTIVATE;
	m_list.SetExtendedStyle(dwStyle);				//设置扩展风格

	int idx = 0;
	m_list.InsertColumn(idx++, _T("componentType"),				LVCFMT_CENTER,	200);
	m_list.InsertColumn(idx++, _T("addr"),						LVCFMT_CENTER,	250);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CStartServerWindow::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	CStartServerLayoutWindow dlg;
	dlg.DoModal();
}

void CStartServerWindow::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
}

void CStartServerWindow::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
}
