#pragma once
#include "afxcmn.h"


// StatusWindow dialog

class StatusWindow : public CDialog
{
	DECLARE_DYNAMIC(StatusWindow)

public:
	StatusWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~StatusWindow();

	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_STATUS };
	void autoWndSize();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_statusList;
};
