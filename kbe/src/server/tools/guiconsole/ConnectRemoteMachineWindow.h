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

	void saveIpMapping();
	void saveHistory();
	void loadIpMapping();
	void loadHistory();
	CString getCurrentHost();
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void updateIpMapping(const CString& host);
	void updateMappingListCtrl();

public:
	afx_msg void OnBnClickedOk();
	CIPAddressCtrl m_ip;
	CEdit m_port;
	std::deque<CString> m_historyCommand;
	CListBox m_log;

	// ip mapping
	CIPAddressCtrl m_lan_ip;
	CIPAddressCtrl m_internet_ip;
	CListBox m_mappinglog;
	std::multimap<CString, CString> m_ipMapping;

	afx_msg void OnLbnDblclkList1();
	afx_msg void OnBnClickedAddIpmapping();
	afx_msg void OnBnClickedDelIpmapping();
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnLbnDblclkList2();
};
