// LogWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "LogWindow.h"
#include "guiconsoleDlg.h"
#include "../../../server/tools/message_log/messagelog_interface.hpp"
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
	DDX_Control(pDX, IDC_APP_LIST1, m_componentlist);
	DDX_Control(pDX, IDC_MSGTYPE_LIST2, m_msgTypeList);
	DDX_Control(pDX, IDC_STATIC_OPTION, m_optiongroup);
	DDX_Control(pDX, IDC_STATIC_APPID, m_appIDstatic);
	DDX_Control(pDX, IDC_APPID_EDIT, m_appIDEdit);
	DDX_Control(pDX, IDC_LOG_LIST1, m_loglist);
	DDX_Control(pDX, IDC_SPIN1, m_showOptionWindow);
}

BOOL CLogWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_componentlist.AddString(L"cellapp");
	m_componentlist.AddString(L"baseapp");
	m_componentlist.AddString(L"cellappmgr");
	m_componentlist.AddString(L"baseappmgr");
	m_componentlist.AddString(L"loginapp");
	m_componentlist.AddString(L"dbmgr");

	for(int i=0; i< m_componentlist.GetCount(); i++)
	{
		CString s;
		m_componentlist.GetText(i, s);

		if(s == L"cellapp" || s == L"baseapp")
			m_componentlist.SetCheck(i, 1);
	}

	m_msgTypeList.AddString(L"PRINT");
	m_msgTypeList.AddString(L"ERROR");
	m_msgTypeList.AddString(L"DEBUG");
	m_msgTypeList.AddString(L"INFO");
	m_msgTypeList.AddString(L"WARNING");
	m_msgTypeList.AddString(L"CRITICAL");
	m_msgTypeList.AddString(L"S_ERR");
	m_msgTypeList.AddString(L"S_DBG");
	m_msgTypeList.AddString(L"S_WARN");
	m_msgTypeList.AddString(L"S_INFO");
	m_msgTypeList.AddString(L"S_NORM");

	for(int i=0; i< m_msgTypeList.GetCount(); i++)
	{
		m_msgTypeList.SetCheck(i, 1);
	}

	pulling = false;
	m_startShowOptionWnd = false;

	CRect rect;
	GetClientRect(&rect);
	m_edit_height = int(rect.bottom * 0.3);
	SetTimer(10, 10, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CLogWindow::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CLogWindow::OnDeltaposSpin1)
END_MESSAGE_MAP()

void CLogWindow::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
	
	CRect rect;
	GetClientRect(&rect);

	if(this->IsWindowVisible())
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point); 

		if(m_startShowOptionWnd)
		{
			if(m_edit_height < int(rect.bottom * 0.3))
			{
				m_edit_height += 20.0f;
				autoWndSize();
			}
			else
			{
				m_edit_height = int(rect.bottom * 0.3);
			}
		}
		else
		{
			if(m_edit_height > 10.0f)
			{
				m_edit_height -= 15.0f;
				autoWndSize();
			}
			else
			{
				m_edit_height = 10.0f;
			}
		}
	}
	else
	{
		m_edit_height = int(rect.bottom * 0.3);
	}
}

void CLogWindow::autoWndSize()
{
	CRect rect, rect1;
	GetClientRect(&rect);

	float addHeight = m_edit_height > 10.0f ? 0.0f : -100000.f;

	m_autopull.MoveWindow(int(rect.right * 0.85) + 3, int(rect.bottom * 0.7), rect.right / 9, int(rect.bottom * 0.05) + addHeight, TRUE);

	m_componentlist.MoveWindow(3, int(rect.bottom * 0.7), rect.right / 5, int(rect.bottom * 0.3) + addHeight, TRUE);
	m_msgTypeList.MoveWindow(rect.right / 5 + 3, int(rect.bottom * 0.7), rect.right / 7, int(rect.bottom * 0.3) + addHeight, TRUE);

	m_optiongroup.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3, int(rect.bottom * 0.7), rect.right / 5, int(rect.bottom * 0.3) + addHeight, TRUE);
	m_appIDstatic.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5, int(rect.bottom * 0.7) + 15,  int(rect.right / 5 * 0.3), int(rect.bottom * 0.03) + addHeight, TRUE);
	m_appIDEdit.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5 +  int((rect.right / 5 * 0.3)), int(rect.bottom * 0.7) + 15 + addHeight,  int(rect.right / 5 * 0.6), int(rect.bottom * 0.04) + addHeight, TRUE);

	m_loglist.MoveWindow(2, 3, rect.right, rect.bottom - m_edit_height - 10, TRUE);

	m_showOptionWindow.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5 +  int((rect.right / 5 * 0.3)), rect.bottom - 7,  int(rect.right / 7 * 0.6), 15, TRUE);
}

// CLogWindow message handlers
void CLogWindow::onReceiveRemoteLog(std::string str)
{
	if(str.size() <= 0)
		return;
	
	CString s;
	wchar_t* wstr = KBEngine::strutil::char2wchar(str.c_str());
	s = wstr;
	free(wstr);
	s.Replace(L"\n", L"");
	s.Replace(L"\r\n", L"");
	s.Replace(L"\n\r", L"");
	s.Replace(L"\r", L"");

	if(s.Find(L"WARNING") >= 0 || s.Find(L"S_WARN") >= 0)
		m_loglist.AddString(s, RGB(0, 0, 0), RGB(255, 165, 0));
	else if(s.Find(L"ERROR") >= 0 || s.Find(L"S_ERR") >= 0)
		m_loglist.AddString(s, RGB(0, 0, 0), RGB(255, 0, 0));
	else if(s.Find(L"CRITICAL") >= 0)
		m_loglist.AddString(s, RGB(0, 0, 0), RGB(100, 149, 237));
	else if(s.Find(L"S_DBG") >= 0 || s.Find(L"S_NORM") >= 0 || s.Find(L"S_INFO") >= 0)
		m_loglist.AddString(s, RGB(0, 0, 0), RGB(237, 237,237));
	else
		m_loglist.AddString(s, RGB(80, 80, 80), RGB(237, 237,237));
}

void CLogWindow::onConnectStatus(bool success, KBEngine::Network::Address addr)
{
	pulling = !success;
	pullLogs(addr);
}

void CLogWindow::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	// 请求服务器拉取日志
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	
	HTREEITEM item = dlg->hasCheckApp(MESSAGELOG_TYPE);
	if(item == NULL)
	{
		::AfxMessageBox(L"messagelog no select!");
		return;
	}

	dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	Network::Address addr = dlg->getTreeItemAddr(item);
	pullLogs(addr);
}

void CLogWindow::pullLogs(KBEngine::Network::Address addr)
{
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	Network::Channel* pChannel = dlg->networkInterface().findChannel(addr);
	if(pChannel == NULL)
	{
		::AfxMessageBox(L"messagelog is error!");
		return;
	}

	if(!pulling)
	{
		Network::Bundle bundle;
		bundle.newMessage(MessagelogInterface::registerLogWatcher);

		bundle << getSelLogTypes();
	
		CString apporder;
		m_appIDEdit.GetWindowTextW(apporder);

		char* cs = KBEngine::strutil::wchar2char(apporder.GetBuffer(0));
		COMPONENT_ORDER order = atoi(cs);
		free(cs);

		bundle << order;

		int8 count = 0;
		std::vector<KBEngine::COMPONENT_TYPE> vec = getSelComponents();
		count = vec.size();
		bundle << count;
		std::vector<KBEngine::COMPONENT_TYPE>::iterator iter = vec.begin();
		for(; iter != vec.end(); iter++)
		{
			bundle << (*iter);
		}

		bool first = m_loglist.GetCount() <= 0;
		bundle << first;
		bundle.send(dlg->networkInterface(), pChannel);

		m_autopull.SetWindowTextW(L"stop");
	}
	else
	{
		m_autopull.SetWindowTextW(L"pull");

		Network::Bundle bundle;
		bundle.newMessage(MessagelogInterface::deregisterLogWatcher);
		bundle.send(dlg->networkInterface(), pChannel);
	}

	pulling = !pulling;
}

std::vector<KBEngine::COMPONENT_TYPE> CLogWindow::getSelComponents()
{
	std::vector<KBEngine::COMPONENT_TYPE> vec;
	for(int i=0; i<m_componentlist.GetCount(); i++)
	{
		if(m_componentlist.GetCheck(i))
		{
			CString s;
			m_componentlist.GetText(i, s);

			if(s == "cellapp")
			{
				vec.push_back(KBEngine::CELLAPP_TYPE);
			}
			else if(s == "baseapp")
			{
				vec.push_back(KBEngine::BASEAPP_TYPE);
			}
			else if(s == "cellappmgr")
			{
				vec.push_back(KBEngine::CELLAPPMGR_TYPE);
			}
			else if(s == "baseappmgr")
			{
				vec.push_back(KBEngine::BASEAPPMGR_TYPE);
			}
			else if(s == "loginapp")
			{
				vec.push_back(KBEngine::LOGINAPP_TYPE);
			}
			else if(s == "dbmgr")
			{
				vec.push_back(KBEngine::DBMGR_TYPE);
			}
		}
	}

	return vec;
}

KBEngine::uint32 CLogWindow::getSelLogTypes()
{
	KBEngine::uint32 types = 0;
	for(int i=0; i<m_msgTypeList.GetCount(); i++)
	{
		if(m_msgTypeList.GetCheck(i))
		{
			CString s;
			m_msgTypeList.GetText(i, s);

			if(s == "PRINT")
			{
				types |= KBELOG_PRINT;
			}
			else if(s == "ERROR")
			{
				types |= KBELOG_ERROR;
			}
			else if(s == "DEBUG")
			{
				types |= KBELOG_DEBUG;
			}
			else if(s == "INFO")
			{
				types |= KBELOG_INFO;
			}
			else if(s == "WARNING")
			{
				types |= KBELOG_WARNING;
			}
			else if(s == "CRITICAL")
			{
				types |= KBELOG_CRITICAL;
			}
			else if(s == "S_INFO")
			{
				types |= KBELOG_SCRIPT_INFO;
			}
			else if(s == "S_ERR")
			{
				types |= KBELOG_SCRIPT_ERROR;
			}
			else if(s == "S_DBG")
			{
				types |= KBELOG_SCRIPT_DEBUG;
			}
			else if(s == "S_WARN")
			{
				types |= KBELOG_SCRIPT_WARNING;
			}
			else if(s == "S_NORM")
			{
				types |= KBELOG_SCRIPT_NORMAL;
			}
		}
	}

	return types;
}

void CLogWindow::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	m_startShowOptionWnd = !m_startShowOptionWnd;
}
