// TimingLengthWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "TimingLengthWindow.h"


// CTimingLengthWindow dialog

IMPLEMENT_DYNAMIC(CTimingLengthWindow, CDialog)

CTimingLengthWindow::CTimingLengthWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CTimingLengthWindow::IDD, pParent)
	, m_timingLength(0)
{

}

CTimingLengthWindow::~CTimingLengthWindow()
{
}

void CTimingLengthWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_timingLength);
}


BEGIN_MESSAGE_MAP(CTimingLengthWindow, CDialog)
	ON_BN_CLICKED(IDOK, &CTimingLengthWindow::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CTimingLengthWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	this->SetDlgItemInt(IDC_EDIT1, 10);

	CString s;
	wchar_t* ws = KBEngine::strutil::char2wchar(m_profileName.c_str());
	s = ws;
	free(ws);
	this->SetDlgItemTextW(IDC_EDIT2, s.GetBuffer(0));
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// CTimingLengthWindow message handlers

void CTimingLengthWindow::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString s;
	this->GetDlgItemTextW(IDC_EDIT2, s);
	char* cs = KBEngine::strutil::wchar2char(s.GetBuffer(0));
	m_profileName = cs;
	free(cs);
	OnOK();
}
