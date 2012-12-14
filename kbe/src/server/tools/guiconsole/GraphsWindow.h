#pragma once
#include "LineChartCtrl.h"

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

	void resetPlot();

	void startPlot();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CLineChartCtrl m_plot;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
