#pragma once


// CSpaceViewWindow dialog

class CSpaceViewWindow : public CDialog
{
	DECLARE_DYNAMIC(CSpaceViewWindow)

public:
	CSpaceViewWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpaceViewWindow();

// Dialog Data
	enum { IDD = IDD_SPACEVIEW };
	void autoWndSize();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
