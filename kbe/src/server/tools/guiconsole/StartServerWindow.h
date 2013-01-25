#pragma once
#include "afxcmn.h"


// CStartServerWindow dialog

class CStartServerWindow : public CDialog
{
	DECLARE_DYNAMIC(CStartServerWindow)

public:
	CStartServerWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStartServerWindow();

// Dialog Data
	enum { IDD = IDD_STARTSERVER };
	
	BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
};
