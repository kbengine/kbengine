
// guiconsoleDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "DebugWindow.h"
#include "LogWindow.h"
#include "StatusWindow.h"
#include "ProfileWindow.h"
#include "WatcherWindow.h"
#include "SpaceViewWindow.h"
#include "GraphsWindow.h"
#include "ConnectRemoteMachineWindow.h"
#include "thread/threadpool.hpp"
#include <sstream>

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
	Mercury::EventDispatcher & mainDispatcher()				{ return _dispatcher; }
	Mercury::NetworkInterface & networkInterface()			{ return _networkInterface; }
	HTREEITEM hasCheckApp(COMPONENT_TYPE type);

	void autoWndSize();
	void updateTree();

	void loadHistory();
	void saveHistory();
	
	void connectTo();

	void autoShowWindow();
	void closeCurrTreeSelChannel();
	Mercury::Address getTreeItemAddr(HTREEITEM hItem);
	COMPONENT_TYPE getTreeItemComponent(HTREEITEM hItem);
	
	bool hasTreeComponent(Components::ComponentInfos& cinfos);

	void onReceiveRemoteLog(std::string str);

	COMPONENT_TYPE componentType()const { return _componentType; }
	COMPONENT_ID componentID()const { return _componentID; }

	void updateFindTreeStatus();

	void clearTree(){
		m_tree.DeleteAllItems();
		m_statusWnd.m_statusList.DeleteAllItems();
	}

	void reqQueryWatcher(std::string paths);
	void onReceiveWatcherData(MemoryStream& s);

	void onReceiveProfileData(MemoryStream& s);

	bool startProfile(std::string name, int8 type, uint32 timinglen);
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CTabCtrl m_tab;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CTreeCtrl m_tree;
	afx_msg void OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	
	afx_msg void OnMenu_Update();

	afx_msg void OnToolBar_Find();
	afx_msg void OnToolBar_StartServer();
	afx_msg void OnToolBar_StopServer();
private:
	COMPONENT_TYPE _componentType;
	COMPONENT_ID _componentID;
	Mercury::EventDispatcher _dispatcher;
	Mercury::NetworkInterface _networkInterface;
	CDebugWindow m_debugWnd;
	CLogWindow m_logWnd;
	StatusWindow m_statusWnd;

	CProfileWindow m_profileWnd;
	CWatcherWindow m_watcherWnd;
	CSpaceViewWindow m_spaceViewWnd;
	CGraphsWindow	m_graphsWindow;

	bool m_isInit;
	std::deque<CString> m_historyCommand;
	int8 m_historyCommandIndex;
	bool m_isUsingHistroy;
	CToolBar m_ToolBar;
	CImageList m_ImageList;

	// 线程池
	thread::ThreadPool threadPool_;	
public:
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnConnectRemoteMachine();
	afx_msg void OnHelpAbout();
	afx_msg void OnClose();
};
