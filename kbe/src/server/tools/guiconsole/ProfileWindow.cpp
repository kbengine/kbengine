// ProfileWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "ProfileWindow.h"
#include "TimingLengthWindow.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
// CProfileWindow dialog

IMPLEMENT_DYNAMIC(CProfileWindow, CDialog)

std::string profilename;

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
	DDX_Control(pDX, IDC_RESULT, m_results);
}

BOOL CProfileWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	DWORD dwStyle = m_profileShowList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_ONECLICKACTIVATE;
	m_profileShowList.SetExtendedStyle(dwStyle);				//设置扩展风格

	std::stringstream ss;
	ss << KBEngine::genUUID64();
	ss << (DWORD)this;
	profilename = ss.str();

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
	
	//m_profileShowList.MoveWindow(2, 3, rect.right, rect.bottom - int(rect.bottom * 0.05) - 5, TRUE);
	m_results.MoveWindow(2, 3, rect.right, rect.bottom - int(rect.bottom * 0.05) - 5, TRUE);
}


BEGIN_MESSAGE_MAP(CProfileWindow, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CProfileWindow::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CProfileWindow::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CProfileWindow::OnBnClickedButton3)
END_MESSAGE_MAP()


// CProfileWindow message handlers

void CProfileWindow::OnBnClickedButton1()
{
	m_pyprofile.EnableWindow(FALSE);
	m_cprofile.EnableWindow(FALSE);
	m_eventprofile.EnableWindow(FALSE);

	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);

	CTimingLengthWindow wnd;
	wnd.m_profileName = profilename;
	if(wnd.DoModal() == IDOK)
	{
		if(dlg->startProfile(wnd.m_profileName, 0, wnd.m_timingLength))
		{
			m_results.SetWindowText(L"please wait for the result.");
			return;
		}
	}

	m_pyprofile.EnableWindow(TRUE);
	m_cprofile.EnableWindow(TRUE);
	m_eventprofile.EnableWindow(TRUE);
	::AfxMessageBox(L"please select the baseapp|cellapp.");
}

void CProfileWindow::OnBnClickedButton2()
{
	m_pyprofile.EnableWindow(FALSE);
	m_cprofile.EnableWindow(FALSE);
	m_eventprofile.EnableWindow(FALSE);
	
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);

	CTimingLengthWindow wnd;
	wnd.m_profileName = profilename;
	if(wnd.DoModal() == IDOK)
	{
		if(dlg->startProfile(wnd.m_profileName, 1, wnd.m_timingLength))
		{
			m_results.SetWindowText(L"please wait for the result.");
			return;
		}
	}

	m_pyprofile.EnableWindow(TRUE);
	m_cprofile.EnableWindow(TRUE);
	m_eventprofile.EnableWindow(TRUE);
	::AfxMessageBox(L"please select the baseapp|cellapp.");
}

void CProfileWindow::OnBnClickedButton3()
{
	m_pyprofile.EnableWindow(FALSE);
	m_cprofile.EnableWindow(FALSE);
	m_eventprofile.EnableWindow(FALSE);
	
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);

	CTimingLengthWindow wnd;
	wnd.m_profileName = profilename;
	if(wnd.DoModal() == IDOK)
	{
		if(dlg->startProfile(wnd.m_profileName, 2, wnd.m_timingLength))
		{
			m_results.SetWindowText(L"please wait for the result.");
			return;
		}
	}

	m_pyprofile.EnableWindow(TRUE);
	m_cprofile.EnableWindow(TRUE);
	m_eventprofile.EnableWindow(TRUE);
	::AfxMessageBox(L"please select the baseapp|cellapp.");
}

void CProfileWindow::onReceiveData(KBEngine::int8 type, std::string& data)
{
	m_pyprofile.EnableWindow(TRUE);
	m_cprofile.EnableWindow(TRUE);
	m_eventprofile.EnableWindow(TRUE);

	switch(type)
	{
	case 0:	// pyprofile
		onReceivePyProfileData(data);
		break;
	case 1:	// cprofile
		onReceiveCProfileData(data);
		break;
	case 2:	// eventprofile
		onReceiveEventProfileData(data);
		break;
	default:
		ERROR_MSG(boost::format("CProfileWindow::onReceiveData: type(%1%) not support!\n") % 
			type);
		break;
	};
}

void CProfileWindow::onReceivePyProfileData(std::string& data)
{
	CString s;
	wchar_t* ws = KBEngine::strutil::char2wchar(data.c_str());
	s = ws;
	s.Replace(L"\n", L"\r\n");
	free(ws);

	m_results.SetWindowText(s);
}

void CProfileWindow::onReceiveCProfileData(std::string& data)
{
}

void CProfileWindow::onReceiveEventProfileData(std::string& data)
{
}