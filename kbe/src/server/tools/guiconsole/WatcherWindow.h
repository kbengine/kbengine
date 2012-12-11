#pragma once
#include "afxcmn.h"
#include "cstdkbe/memorystream.hpp"
#include "helper/watcher.hpp"

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
	void addHeader(std::string name);
	void addItem(KBEngine::WatcherObject* wo);
	void changePath(std::string path);
	void addPath(std::string path);

	void clearAllData(bool clearTree = true);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_tree;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CListCtrl m_status;
};
