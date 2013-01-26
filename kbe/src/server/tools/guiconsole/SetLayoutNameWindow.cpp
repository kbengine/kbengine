// SetLayoutNameWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "SetLayoutNameWindow.h"


// CSetLayoutNameWindow dialog

IMPLEMENT_DYNAMIC(CSetLayoutNameWindow, CDialog)

CSetLayoutNameWindow::CSetLayoutNameWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CSetLayoutNameWindow::IDD, pParent)
	, m_name(_T(""))
{

}

CSetLayoutNameWindow::~CSetLayoutNameWindow()
{
}

void CSetLayoutNameWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}


BEGIN_MESSAGE_MAP(CSetLayoutNameWindow, CDialog)
	ON_BN_CLICKED(IDOK, &CSetLayoutNameWindow::OnBnClickedOk)
END_MESSAGE_MAP()


// CSetLayoutNameWindow message handlers
BOOL CSetLayoutNameWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_edit.SetWindowTextW(L"default_layout");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSetLayoutNameWindow::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();

	m_edit.GetWindowText(m_name);
}
