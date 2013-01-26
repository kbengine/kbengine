#pragma once
#include "afxwin.h"


// CSetLayoutNameWindow dialog

class CSetLayoutNameWindow : public CDialog
{
	DECLARE_DYNAMIC(CSetLayoutNameWindow)

public:
	CSetLayoutNameWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetLayoutNameWindow();

// Dialog Data
	enum { IDD = IDD_SET_LAYOUT_NAME };

	BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_name;
	afx_msg void OnBnClickedOk();
	CEdit m_edit;
};
