#pragma once
#include "afxwin.h"
#include "ColorListBox.h"
#include "MultiLineListBox.h"
#include "common/common.hpp"
#include "network/address.hpp"
#include "afxcmn.h"

// CLogWindow dialog

class CLogWindow : public CDialog
{
	DECLARE_DYNAMIC(CLogWindow)

public:
	CLogWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogWindow();

// Dialog Data
	enum { IDD = IDD_LOG };
	void autoWndSize();
	virtual BOOL OnInitDialog();

	KBEngine::uint32 getSelLogTypes();
	std::vector<KBEngine::COMPONENT_TYPE> getSelComponents();

	void onReceiveRemoteLog(std::string str);

	void onConnectStatus(bool success, KBEngine::Network::Address addr);

	void pullLogs(KBEngine::Network::Address addr);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_autopull;
	CCheckListBox m_componentlist;
	CCheckListBox m_msgTypeList;
	CStatic m_optiongroup;
	CStatic m_appIDstatic;
	CEdit m_appIDEdit;
	afx_msg void OnBnClickedButton1();
	CMultiLineListBox m_loglist;

	bool pulling;
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	float m_edit_height;
	CSpinButtonCtrl m_showOptionWindow;
	bool m_startShowOptionWnd;
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
};
