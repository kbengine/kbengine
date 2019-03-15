// LogWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "LogWindow.h"
#include "guiconsoleDlg.h"
#include "common/common.h"
#include "../../../server/tools/logger/logger_interface.h"
// CLogWindow dialog

#pragma warning(disable:4244)
IMPLEMENT_DYNAMIC(CLogWindow, CDialog)

CString  state_flags[8] = {
	L"↖",
	L"↑",
	L"↗",
	L"→",
	L"↘ ",
	L"↓",
	L"↙",
	L"←",
};

int state_flags_idx = 0;

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
	DDX_Control(pDX, IDC_STATIC_APPID1, m_appIDstatic1);
	DDX_Control(pDX, IDC_LOG_DATE_STATIC, m_dateStatic);
	DDX_Control(pDX, IDC_LOG_FIND, m_findStatic);
	DDX_Control(pDX, IDC_APPID_EDIT, m_appIDEdit);
	DDX_Control(pDX, IDC_APPID_EDIT1, m_appIDEdit1);
	DDX_Control(pDX, IDC_LOG_DATE, m_dateEdit);
	DDX_Control(pDX, IDC_LOG_FINDSTR, m_findEdit);
	DDX_Control(pDX, IDC_LOG_LIST1, m_loglist);
	DDX_Control(pDX, IDC_SPIN1, m_showOptionWindow);
	DDX_Control(pDX, IDC_ERROR, m_errBtn);
	DDX_Control(pDX, IDC_WARNING, m_warnBtn);
	DDX_Control(pDX, IDC_INFO, m_infoBtn);
	DDX_Control(pDX, IDC_MFCBUTTON1, m_clear);
	DDX_Control(pDX, IDC_BUTTON2, m_pullonce);
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

	SetTimer(1, 10, NULL);
	SetTimer(2, 100, NULL);

	m_errCount = 0;
	m_warnCount = 0;
	m_infoCount = 0;

	m_errChecked = TRUE;
	m_warnChecked = TRUE;
	m_infoChecked = TRUE;


	hInfoBitmap = LoadBitmap(AfxGetInstanceHandle(),   
			MAKEINTRESOURCE(IDB_INFO));

	hInfoBitmap1 = LoadBitmap(AfxGetInstanceHandle(),   
			MAKEINTRESOURCE(IDB_INFO1));

	hWarnBitmap = LoadBitmap(AfxGetInstanceHandle(),   
		MAKEINTRESOURCE(IDB_WARNING));

	hWarnBitmap1 = LoadBitmap(AfxGetInstanceHandle(),   
		MAKEINTRESOURCE(IDB_WARNING1));

	hErrBitmap = LoadBitmap(AfxGetInstanceHandle(),   
		MAKEINTRESOURCE(IDB_ERROR));

	hErrBitmap1 = LoadBitmap(AfxGetInstanceHandle(),   
		MAKEINTRESOURCE(IDB_ERROR1));

	updateLogBtnStatus(false);

	isfind_= false;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLogWindow::updateLogBtnStatus(bool updateList)
{
	m_infoBtn.SetBitmap(m_infoChecked ? hInfoBitmap : hInfoBitmap1);  
	m_warnBtn.SetBitmap(m_warnChecked? hWarnBitmap : hWarnBitmap1);  
	m_errBtn.SetBitmap(m_errChecked ? hErrBitmap : hErrBitmap1); 

	CString s;
	s.Format(L"%d", m_warnCount);
	m_warnBtn.SetWindowTextW(s);

	s.Format(L"%d", m_infoCount);
	m_infoBtn.SetWindowTextW(s);

	s.Format(L"%d", m_errCount);
	m_errBtn.SetWindowTextW(s);

	if(updateList)
	{
		m_loglist.Clear();

		std::list<std::string>::iterator iter = m_logs_.begin();
		for(; iter != m_logs_.end(); iter++)
		{
			onReceiveRemoteLog((*iter), false);
		}
	}
}

BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CLogWindow::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CLogWindow::OnDeltaposSpin1)
	ON_BN_CLICKED(IDC_WARNING, &CLogWindow::OnBnClickedWarning)
	ON_BN_CLICKED(IDC_ERROR, &CLogWindow::OnBnClickedError)
	ON_BN_CLICKED(IDC_INFO, &CLogWindow::OnBnClickedInfo)
	ON_BN_CLICKED(IDC_MFCBUTTON1, &CLogWindow::OnBnClickedMfcbutton1)
	ON_NOTIFY(NM_THEMECHANGED, IDC_APP_LIST1, &CLogWindow::OnNMThemeChangedAppList1)
	ON_LBN_SELCHANGE(IDC_APP_LIST1, &CLogWindow::OnLbnSelchangeAppList1)
	ON_LBN_SELCHANGE(IDC_MSGTYPE_LIST2, &CLogWindow::OnLbnSelchangeMsgtypeList2)
	ON_BN_CLICKED(IDC_BUTTON2, &CLogWindow::OnBnClickedButton2)
	ON_STN_CLICKED(IDC_STATIC_APPID1, &CLogWindow::OnStnClickedStaticAppid1)
END_MESSAGE_MAP()

void CLogWindow::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
	
	if(nIDEvent == 1)
	{
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
				if(m_edit_height > 4.0f)
				{
					m_edit_height -= 15.0f;
					autoWndSize();
				}
				else
				{
					m_edit_height = 4.0f;
				}
			}
		}
		else
		{
			m_edit_height = int(rect.bottom * 0.3);
		}
	}
	else
	{
		if(pulling)
			m_autopull.SetWindowTextW(state_flags[(state_flags_idx++) % 8]);
	}
}

void CLogWindow::autoWndSize()
{
	CRect rect, rect1;
	GetClientRect(&rect);

	float addHeight = m_edit_height > 4.0f ? 0.0f : -100000.f;

	m_autopull.MoveWindow(int(rect.right * 0.85) + 3, int(rect.bottom * 0.7), rect.right / 9, int(rect.bottom * 0.05) + addHeight, TRUE);
	m_pullonce.MoveWindow(int(rect.right * 0.85) + 3, int(rect.bottom * 0.75), rect.right / 9, int(rect.bottom * 0.05) + addHeight, TRUE);

	m_componentlist.MoveWindow(3, int(rect.bottom * 0.7), rect.right / 5, int(rect.bottom * 0.3) + addHeight, TRUE);
	m_msgTypeList.MoveWindow(rect.right / 5 + 3, int(rect.bottom * 0.7), rect.right / 7, int(rect.bottom * 0.3) + addHeight, TRUE);

	m_optiongroup.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3, int(rect.bottom * 0.7), rect.right / 2.9, int(rect.bottom * 0.3) + addHeight, TRUE);
	m_appIDstatic.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5, int(rect.bottom * 0.7) + 15,  int(rect.right / 5 * 0.5), int(rect.bottom * 0.04) + addHeight, TRUE);
	m_appIDEdit.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 30 +  int((rect.right / 5 * 0.3)), int(rect.bottom * 0.7) + 15 + addHeight,  int(rect.right / 5 * 0.6), int(rect.bottom * 0.04) + addHeight, TRUE);
	m_appIDstatic1.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5, int(rect.bottom * 0.75) + 15,  int(rect.right / 5 * 0.5), int(rect.bottom * 0.04) + addHeight, TRUE);
	m_appIDEdit1.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 30 +  int((rect.right / 5 * 0.3)), int(rect.bottom * 0.75) + 15 + addHeight,  int(rect.right / 5 * 0.6), int(rect.bottom * 0.04) + addHeight, TRUE);
	m_dateStatic.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5, int(rect.bottom * 0.8) + 15,  int(rect.right / 5 * 0.3), int(rect.bottom * 0.03) + addHeight, TRUE);
	m_dateEdit.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5 +  int((rect.right / 5 * 0.3)), int(rect.bottom * 0.8) + 15 + addHeight,  int(rect.right / 3 * 0.8), int(rect.bottom * 0.04) + addHeight, TRUE);
	m_findStatic.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5, int(rect.bottom * 0.85) + 15,  int(rect.right / 5 * 0.3), int(rect.bottom * 0.03) + addHeight, TRUE);
	m_findEdit.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5 +  int((rect.right / 5 * 0.3)), int(rect.bottom * 0.85) + 15 + addHeight,  int(rect.right / 3 * 0.8), int(rect.bottom * 0.04) + addHeight, TRUE);

	m_loglist.MoveWindow(2, 3, rect.right, rect.bottom - m_edit_height - 10, TRUE);

	m_showOptionWindow.MoveWindow(rect.right / 5 + 3 + rect.right / 7 + 3 + 5 +  int((rect.right / 5 * 0.3)), rect.bottom - 7,  int(rect.right / 7 * 0.6), 15, TRUE);

	m_errBtn.MoveWindow(rect.right - 180, rect.bottom - 12,  60, 12, TRUE);
	m_warnBtn.MoveWindow(rect.right - 120, rect.bottom - 12,  60, 12, TRUE);
	m_infoBtn.MoveWindow(rect.right - 60, rect.bottom - 12,  60, 12, TRUE);

	m_clear.MoveWindow(rect.right - 200, rect.bottom - 11,  10, 10, TRUE);
}

// CLogWindow message handlers
void CLogWindow::onReceiveRemoteLog(std::string str, bool fromServer)
{
	if(str.size() <= 0)
		return;
	
	if(fromServer)
		m_logs_.push_back(str);

	std::wstring wstr;
	KBEngine::strutil::utf82wchar(str.c_str(), wstr);

	CString s(wstr.c_str());
	s.Replace(L"\n", L"");
	s.Replace(L"\r\n", L"");
	s.Replace(L"\n\r", L"");
	s.Replace(L"\r", L"");

	std::vector<std::wstring> logSplit;
	KBEngine::strutil::kbe_split<wchar_t>(s.GetBuffer(0), L' ', logSplit);

	if(logSplit.size() == 0)
		return;

	s.Replace((logSplit[2] + L" " + logSplit[3]).c_str(), L"");

	if(logSplit[0] == L"WARNING" || logSplit[0] == L"S_WARN")
	{
		if(fromServer)
			m_warnCount++;

		if(m_warnChecked)
			m_loglist.AddString(s, RGB(0, 0, 0), RGB(255, 165, 0));
	}
	else if(logSplit[0] == L"ERROR" || logSplit[0] == L"S_ERR")
	{
		if(fromServer)
			m_errCount++;

		if(m_errChecked)
			m_loglist.AddString(s, RGB(0, 0, 0), RGB(255, 0, 0));
	}
	else if(logSplit[0] == L"CRITICAL")
	{
		if(fromServer)
			m_errCount++;

		if(m_errChecked)
			m_loglist.AddString(s, RGB(0, 0, 0), RGB(100, 149, 237));
	}
	else if(logSplit[0] == L"S_DBG" || logSplit[0] == L"S_NORM" || logSplit[0] == L"S_INFO")
	{
		if(fromServer)
			m_infoCount++;

		if(m_infoChecked)
			m_loglist.AddString(s, RGB(0, 0, 0), RGB(237, 237,237));
	}
	else
	{
		if(fromServer)
			m_infoCount++;

		if(m_infoChecked)
			m_loglist.AddString(s, RGB(80, 80, 80), RGB(237, 237,237));
	}

	updateLogBtnStatus(false);
}

void CLogWindow::onConnectionState(bool success, KBEngine::Network::Address addr)
{
	OnBnClickedMfcbutton1();
	pulling = !success;
	pullLogs(addr);
}

void CLogWindow::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	// 请求服务器拉取日志
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	
	HTREEITEM item = dlg->hasCheckApp(LOGGER_TYPE);
	if(item == NULL)
	{
		::AfxMessageBox(L"logger no select!");
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
		::AfxMessageBox(L"logger error!");
		return;
	}

	if(!pulling)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(LoggerInterface::registerLogWatcher);

		int32 uid = dlg->getSelTreeItemUID();
		(*pBundle) << uid;

		(*pBundle) << getSelLogTypes();
	
		CString apporder;
		m_appIDEdit.GetWindowTextW(apporder);

		char* cs = KBEngine::strutil::wchar2char(apporder.GetBuffer(0));
		COMPONENT_ORDER order = atoi(cs);
		free(cs);

		(*pBundle) << order;

		m_appIDEdit1.GetWindowTextW(apporder);
		cs = KBEngine::strutil::wchar2char(apporder.GetBuffer(0));
		order = atoi(cs);
		free(cs);

		(*pBundle) << order;

		CString date;
		m_dateEdit.GetWindowTextW(date);
		cs = KBEngine::strutil::wchar2char(date.GetBuffer(0));
		(*pBundle) << cs;
		free(cs);

		CString keystr;
		m_findEdit.GetWindowTextW(keystr);
		cs = KBEngine::strutil::wchar2char(keystr.GetBuffer(0));
		(*pBundle) << cs;
		free(cs);

		int8 count = 0;
		std::vector<KBEngine::COMPONENT_TYPE> vec = getSelComponents();
		count = vec.size();
		(*pBundle) << count;
		std::vector<KBEngine::COMPONENT_TYPE>::iterator iter = vec.begin();
		for(; iter != vec.end(); iter++)
		{
			(*pBundle) << (*iter);
		}

		(*pBundle) << isfind_;

		bool first = m_loglist.GetCount() <= 0;
		(*pBundle) << first;
		pChannel->send(pBundle);

		m_autopull.SetWindowTextW(L"stop");
	}
	else
	{
		m_autopull.SetWindowTextW(L"auto");

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(LoggerInterface::deregisterLogWatcher);
		pChannel->send(pBundle);
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

void CLogWindow::OnBnClickedWarning()
{
	// TODO: Add your control notification handler code here
	m_warnChecked = !m_warnChecked;
	updateLogBtnStatus();
}

void CLogWindow::OnBnClickedError()
{
	// TODO: Add your control notification handler code here
	m_errChecked = !m_errChecked;
	updateLogBtnStatus();
}

void CLogWindow::OnBnClickedInfo()
{
	// TODO: Add your control notification handler code here
	m_infoChecked = !m_infoChecked;
	updateLogBtnStatus();
}

void CLogWindow::OnBnClickedMfcbutton1()
{
	// TODO: Add your control notification handler code here
	m_logs_.clear();

	m_errCount = 0;
	m_warnCount = 0;
	m_infoCount = 0;
	updateLogBtnStatus();
}


void CLogWindow::OnNMThemeChangedAppList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.
	// TODO: Add your control notification handler code here
	*pResult = 0;
	updateSettingToServer();
}

void CLogWindow::OnLbnSelchangeAppList1()
{
	// TODO: Add your control notification handler code here
	updateSettingToServer();
}

void CLogWindow::OnLbnSelchangeMsgtypeList2()
{
	// TODO: Add your control notification handler code here
	updateSettingToServer();
}

void CLogWindow::updateSettingToServer()
{
	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(LoggerInterface::updateLogWatcherSetting);

	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	int32 uid = dlg->getSelTreeItemUID();
	(*pBundle) << uid;

	(*pBundle) << getSelLogTypes();
	
	CString apporder;
	m_appIDEdit.GetWindowTextW(apporder);

	char* cs = KBEngine::strutil::wchar2char(apporder.GetBuffer(0));
	COMPONENT_ORDER order = atoi(cs);
	free(cs);

	(*pBundle) << order;

	m_appIDEdit1.GetWindowTextW(apporder);
	cs = KBEngine::strutil::wchar2char(apporder.GetBuffer(0));
	order = atoi(cs);
	free(cs);

	(*pBundle) << order;

	CString date;
	m_dateEdit.GetWindowTextW(date);
	cs = KBEngine::strutil::wchar2char(date.GetBuffer(0));
	(*pBundle) << cs;
	free(cs);

	CString keystr;
	m_findEdit.GetWindowTextW(keystr);
	cs = KBEngine::strutil::wchar2char(keystr.GetBuffer(0));
	(*pBundle) << cs;
	free(cs);

	int8 count = 0;
	std::vector<KBEngine::COMPONENT_TYPE> vec = getSelComponents();
	count = vec.size();
	(*pBundle) << count;
	std::vector<KBEngine::COMPONENT_TYPE>::iterator iter = vec.begin();
	for(; iter != vec.end(); iter++)
	{
		(*pBundle) << (*iter);
	}

	HTREEITEM item = dlg->hasCheckApp(LOGGER_TYPE);
	if(item == NULL)
	{
		::AfxMessageBox(L"logger no select!");
		Network::Bundle::reclaimPoolObject(pBundle);
		return;
	}

	dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	Network::Address addr = dlg->getTreeItemAddr(item);
	Network::Channel* pChannel = dlg->networkInterface().findChannel(addr);
	if(pChannel == NULL)
	{
		::AfxMessageBox(L"logger error!");
		Network::Bundle::reclaimPoolObject(pBundle);
		return;
	}

	bool first = m_loglist.GetCount() <= 0;
	(*pBundle) << first;
	pChannel->send(pBundle);
}

void CLogWindow::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	OnBnClickedMfcbutton1();
	isfind_ = true;

	if(pulling == false)
	{
		OnBnClickedButton1();
		OnBnClickedButton1();
	}
	else
	{
		OnBnClickedButton1();
		OnBnClickedButton1();
		OnBnClickedButton1();
	}

	pulling = false;
	isfind_ = false;
}


void CLogWindow::OnStnClickedStaticAppid1()
{
	// TODO: Add your control notification handler code here
}
