#pragma once
#include "afxwin.h"
#include "ColorListBox.h"
#include "MultiLineListBox.h"
#include "common/common.h"
#include "network/address.h"
#include "afxcmn.h"
#include "afxbutton.h"

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

	void onReceiveRemoteLog(std::string str, bool fromServer = true);

	void onConnectionState(bool success, KBEngine::Network::Address addr);

	void pullLogs(KBEngine::Network::Address addr);

	void updateLogBtnStatus(bool updateList = true);
	void updateSettingToServer();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_autopull;
	CCheckListBox m_componentlist;
	CCheckListBox m_msgTypeList;
	CStatic m_optiongroup;
	CStatic m_appIDstatic, m_appIDstatic1, m_dateStatic, m_findStatic;
	CEdit m_appIDEdit, m_appIDEdit1, m_dateEdit, m_findEdit;
	afx_msg void OnBnClickedButton1();
	CMultiLineListBox m_loglist;

	bool pulling;
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	float m_edit_height;
	CSpinButtonCtrl m_showOptionWindow;
	bool m_startShowOptionWnd;
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);

	CButton m_errBtn;
	CButton m_warnBtn;
	CButton m_infoBtn;

	int m_errCount;
	int m_warnCount;
	int m_infoCount;

	BOOL m_errChecked;
	BOOL m_warnChecked;
	BOOL m_infoChecked;

	HBITMAP   hErrBitmap, hErrBitmap1;
	HBITMAP   hWarnBitmap, hWarnBitmap1;
	HBITMAP   hInfoBitmap, hInfoBitmap1;

	afx_msg void OnBnClickedWarning();
	afx_msg void OnBnClickedError();
	afx_msg void OnBnClickedInfo();
	CMFCButton m_clear;
	afx_msg void OnBnClickedMfcbutton1();
	afx_msg void OnNMThemeChangedAppList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLbnSelchangeAppList1();
	afx_msg void OnLbnSelchangeMsgtypeList2();

	std::list<std::string> m_logs_;
	CButton m_pullonce;
	bool isfind_;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnStnClickedStaticAppid1();
};
