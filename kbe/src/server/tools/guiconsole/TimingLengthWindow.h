#pragma once
#include "afxwin.h"


// CTimingLengthWindow dialog

class CTimingLengthWindow : public CDialog
{
	DECLARE_DYNAMIC(CTimingLengthWindow)

public:
	CTimingLengthWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTimingLengthWindow();

// Dialog Data
	enum { IDD = IDD_TIMINGLENGTH };
	
	BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	UINT m_timingLength;
	std::string m_profileName;
	afx_msg void OnBnClickedOk();
};
