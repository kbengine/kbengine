#pragma once


// CGraphsWindow dialog

class CGraphsWindow : public CDialog
{
	DECLARE_DYNAMIC(CGraphsWindow)

public:
	CGraphsWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphsWindow();

// Dialog Data
	enum { IDD = IDD_GRAPHS };

	virtual BOOL OnInitDialog();

	void autoWndSize();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
