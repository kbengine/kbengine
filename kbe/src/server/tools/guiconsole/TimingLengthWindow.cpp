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
END_MESSAGE_MAP()

BOOL CTimingLengthWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	this->SetDlgItemInt(IDC_EDIT1, 10);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// CTimingLengthWindow message handlers
