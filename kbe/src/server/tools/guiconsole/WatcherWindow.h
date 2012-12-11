#pragma once
#include "afxcmn.h"
#include "cstdkbe/memorystream.hpp"

// CWatcherWindow dialog

class CWatcherWindow : public CDialog
{
	DECLARE_DYNAMIC(CWatcherWindow)

public:
	CWatcherWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWatcherWindow();

// Dialog Data
	enum { IDD = IDD_WATCHER };
	void autoWndSize();

	BOOL OnInitDialog();


	void onReceiveWatcherData(KBEngine::MemoryStream& s);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_tree;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CListCtrl m_status;
};
