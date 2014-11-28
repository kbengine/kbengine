#pragma once
#include "afxwin.h"
#include "ColorListBox.h"
#include "MultiLineListBox.h"
#include "common/common.hpp"

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
};
