#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <string>
#include "common/common.h"

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

	void loadLayouts();
	void saveLayouts();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	CComboBox m_layoutlist;

	struct LAYOUT_ITEM
	{
		std::string componentName;
		std::string addr;
	};

	KBEUnordered_map< std::string, std::vector<LAYOUT_ITEM> > layouts_;
//	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnNMThemeChangedCombo3(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_list1;
};
