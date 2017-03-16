#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "common/memorystream.h"

// CProfileWindow dialog

class CProfileWindow : public CDialog
{
	DECLARE_DYNAMIC(CProfileWindow)

public:
	CProfileWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProfileWindow();

// Dialog Data
	enum { IDD = IDD_PROFILE };
	void autoWndSize();

	BOOL OnInitDialog();

	void onReceiveData(KBEngine::int8 type, KBEngine::MemoryStream& s);

	void onReceivePyProfileData(KBEngine::MemoryStream& s);
	void onReceiveCProfileData(KBEngine::MemoryStream& s);
	void onReceiveEventProfileData(KBEngine::MemoryStream& s);
	void onReceiveNetworkProfileData(KBEngine::MemoryStream& s);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_pyprofile;
	CButton m_cprofile;
	CButton m_eventprofile;
	CListCtrl m_profileShowList;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	CEdit m_results;
	afx_msg void OnBnClickedButton4();
	CButton m_networkprofile;
};
