
// guiconsoleDlg.h : header file
//

#pragma once
#include "cstdkbe/cstdkbe.hpp"
#include "network/endpoint.hpp"
#include "network/common.hpp"
#include "network/channel.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "helper/debug_helper.hpp"
#include "xmlplus/xmlplus.hpp"
#include "cstdkbe/smartpointer.hpp"

using namespace KBEngine;

// CguiconsoleDlg dialog
class CguiconsoleDlg : public CDialog
					//	public Mercury::ChannelTimeOutHandler,
					//	public Mercury::ChannelDeregisterHandler
{
// Construction
public:
	CguiconsoleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GUICONSOLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	Mercury::EventDispatcher _dispatcher;
	Mercury::NetworkInterface _networkInterface;
	
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
