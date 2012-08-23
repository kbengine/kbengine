
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
#include "afxcmn.h"
#include "DebugWindow.h"

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
public:
	/** 服务器执行指令完毕回显 */
	void onExecScriptCommandCB(Mercury::Channel* pChannel, std::string& command);

	BOOL PreTranslateMessage(MSG* pMsg);

	void historyCommandCheck();
	CString getHistoryCommand(bool isNextCommand);
	void commitPythonCommand(CString strCommand);
	Mercury::EventDispatcher & getMainDispatcher()				{ return _dispatcher; }
	Mercury::NetworkInterface & getNetworkInterface()			{ return _networkInterface; }
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void autoWndSize();
	void updateTree();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CTabCtrl m_tab;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CTreeCtrl m_tree;
	afx_msg void OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenu_connectTo();
	afx_msg void OnMenu_Update();
private:
	COMPONENT_TYPE _componentType, _debugComponentType;
	COMPONENT_ID _componentID;
	Mercury::EventDispatcher _dispatcher;
	Mercury::NetworkInterface _networkInterface;
	Mercury::Address _currAddr;
	CDebugWindow m_debugWnd;
	bool m_isInit;
	std::deque<CString> m_historyCommand;
	int8 m_historyCommandIndex;
	bool m_isUsingHistroy;
};
