#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CConnectRemoteMachineWindow dialog

class CConnectRemoteMachineWindow : public CDialog
{
	DECLARE_DYNAMIC(CConnectRemoteMachineWindow)

public:
	CConnectRemoteMachineWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConnectRemoteMachineWindow();

// Dialog Data
	enum { IDD = IDD_DIALOG_REMOTE_CONNECT };
	
	void saveHistory();
	void loadHistory();
	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CIPAddressCtrl m_ip;
	CEdit m_port;
	std::deque<CString> m_historyCommand;
	CListBox m_log;
	afx_msg void OnLbnDblclkList1();
};
