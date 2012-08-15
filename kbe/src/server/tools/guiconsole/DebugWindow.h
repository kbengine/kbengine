#pragma once
#include "afxwin.h"
#include "EditSendbuffer.h"

// CDebugWindow dialog

class CDebugWindow : public CDialog
{
	DECLARE_DYNAMIC(CDebugWindow)

public:
	CDebugWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDebugWindow();

// Dialog Data
	enum { IDD = IDD_DEBUG };
	
	CEdit* displaybufferWnd(){ return &m_displaybuffer; }
	CEdit* sendbufferWnd(){ return &m_sendbuffer; }
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_displaybuffer;
	CEditSendbuffer m_sendbuffer;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();

	void autoWndSize();
};
