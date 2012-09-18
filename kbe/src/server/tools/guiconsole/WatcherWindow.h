#pragma once


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
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
