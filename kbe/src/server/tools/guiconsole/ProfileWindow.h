#pragma once
#include "afxwin.h"
#include "afxcmn.h"


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

	void onReceiveData(KBEngine::int8 type, std::string& data);

	void onReceivePyProfileData(std::string& data);
	void onReceiveCProfileData(std::string& data);
	void onReceiveEventProfileData(std::string& data);
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
};
