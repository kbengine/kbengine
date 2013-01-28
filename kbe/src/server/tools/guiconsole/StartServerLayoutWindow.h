#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CStartServerLayoutWindow dialog

class CStartServerLayoutWindow : public CDialog
{
	DECLARE_DYNAMIC(CStartServerLayoutWindow)

public:
	CStartServerLayoutWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStartServerLayoutWindow();

// Dialog Data
	enum { IDD = IDD_STARTSERVERLAYOUT };

	BOOL OnInitDialog();

	void saveHistory();
	void loadHistory();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	afx_msg void OnBnClickedButton1();
	CListBox m_componentlist;
	CEdit m_port;

	std::deque<CString> m_historyCommand;
	CListBox m_log;
	CIPAddressCtrl m_ip;
	afx_msg void OnBnClickedButton2();
	CComboBox m_layoutlist;
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnHdnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton4();
};
