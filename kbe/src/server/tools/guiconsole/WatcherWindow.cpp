// WatcherWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "WatcherWindow.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "helper/watcher.hpp"

// CWatcherWindow dialog

IMPLEMENT_DYNAMIC(CWatcherWindow, CDialog)

CWatcherWindow::CWatcherWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CWatcherWindow::IDD, pParent)
{

}

CWatcherWindow::~CWatcherWindow()
{
}

void CWatcherWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
}

BOOL CWatcherWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	::SetTimer(m_hWnd, 1, 1500, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWatcherWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);

	m_tree.MoveWindow(2, 3, rect.right - 2, rect.bottom - 3, TRUE);
}

BEGIN_MESSAGE_MAP(CWatcherWindow, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWatcherWindow message handlers

void CWatcherWindow::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
	if(!this->IsWindowVisible())
		return;

	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	dlg->reqQueryWatcher("");
}

void CWatcherWindow::onReceiveWatcherData(KBEngine::MemoryStream& s)
{
	KBEngine::WatcherPaths watcherPaths;

	while(s.opsize() > 0)
	{
		std::string path;
		s >> path;

		std::string name;
		s >> name;

		KBEngine::WATCHER_ID id = 0;
		s >> id;

		KBEngine::WATCHERTYPE type;
		s >> type;

		watcherPaths.addWatcherFromStream(path, name, id, type, &s);
	};
}
