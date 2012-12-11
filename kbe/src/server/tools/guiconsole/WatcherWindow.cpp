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
	DDX_Control(pDX, IDC_LIST1, m_status);
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

	m_tree.MoveWindow(2, 3, int((rect.right - 2) * 0.3), rect.bottom - 3, TRUE);
	m_status.MoveWindow(2 + int((rect.right - 2) * 0.3), 3, int((rect.right - 2) * 0.7), rect.bottom - 3, TRUE);
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

	m_tree.DeleteAllItems();
	m_status.DeleteAllItems();
	int nColumnCount = m_status.GetHeaderCtrl()->GetItemCount();       
	for (int i=0;i < nColumnCount;i++)
	{
		m_status.DeleteColumn(0);
	}

	//watcherPaths.
	/*
	HTREEITEM hItemRoot;
	TV_INSERTSTRUCT tcitem;
	tcitem.hParent = TVI_ROOT;
	tcitem.hInsertAfter = TVI_LAST;
	tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tcitem.item.pszText = L"root";
	tcitem.item.lParam = 0;
	tcitem.item.iImage = 0;
	tcitem.item.iSelectedImage = 1;
	hItemRoot = m_tree.InsertItem(&tcitem);
	*/

}
