#pragma once
#include "afxcmn.h"
#include "common/memorystream.h"
#include "helper/watcher.h"

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
	void addHeaderShow(CString name);
	
	void addItemShow(KBEngine::WatcherObject* wo);
	void addItem(KBEngine::WatcherObject* wo);
	void changePath(std::string path);
	void addPath(std::string rootpath, std::string path);

	HTREEITEM findAndCreatePathItem(std::string path, HTREEITEM rootItem);
	void clearAllData(bool clearTree = true);

	std::string getCurrSelPath();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_tree;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CListCtrl m_status;
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_statusShow;
};
