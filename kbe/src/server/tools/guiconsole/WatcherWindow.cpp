// WatcherWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "WatcherWindow.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "helper/watcher.hpp"

// CWatcherWindow dialog
bool g_changePath = true;

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

	DWORD dwStyle = m_status.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_ONECLICKACTIVATE;
	m_status.SetExtendedStyle(dwStyle);					//设置扩展风格

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

void CWatcherWindow::addHeader(std::string name)
{
	wchar_t* ws = KBEngine::char2wchar(name.c_str());
	CString s = ws;
	free(ws);
	int nColumnCount = 0;

	if(m_status.GetHeaderCtrl())
	{
		nColumnCount = m_status.GetHeaderCtrl()->GetItemCount();       
		for (int i=0;i < nColumnCount;i++)
		{
			LVCOLUMN lvcol;
			WCHAR str[256];
			memset(str, 0, 256);
			lvcol.mask=LVCF_TEXT|LVCF_WIDTH;
			lvcol.pszText=str;
			lvcol.cchTextMax=256;
			lvcol.cx = i;
			m_status.GetColumn(i, &lvcol);

			if(s == lvcol.pszText)
				return;
		}
	}

	m_status.InsertColumn(nColumnCount, s, LVCFMT_CENTER,	100);
}

void CWatcherWindow::addItem(KBEngine::WatcherObject* wo)
{
	int idx = 0;
	wchar_t* ws = KBEngine::char2wchar(wo->str());
	CString s = ws;
	free(ws);

	ws = KBEngine::char2wchar(wo->name());
	CString s1 = ws;
	free(ws);

	int nColumnCount = m_status.GetHeaderCtrl()->GetItemCount();       
	for (int i=0;i < nColumnCount;i++)
	{
		LVCOLUMN lvcol;
		WCHAR str[1024];
		memset(str, 0, 1024);
		lvcol.mask=LVCF_TEXT|LVCF_WIDTH;
		lvcol.pszText=str;
		lvcol.cchTextMax=1024;
		lvcol.cx = i;
		m_status.GetColumn(i, &lvcol);

		if(s1 == lvcol.pszText)
		{
			idx = i;
			break;
		}
	}

	if(m_status.GetItemCount() <= 0)
		m_status.InsertItem(0, L"");

	m_status.SetItemText(0, idx, s);
}

void CWatcherWindow::clearAllData(bool clearTree)
{
	m_tree.DeleteAllItems();
	m_status.DeleteAllItems();
	if(m_status.GetHeaderCtrl())
	{
		int nColumnCount = m_status.GetHeaderCtrl()->GetItemCount();       
		for (int i=0;i < nColumnCount;i++)
		{
			m_status.DeleteColumn(0);
		}
	}
}

void CWatcherWindow::changePath(std::string path)
{
	clearAllData(false);
}

void CWatcherWindow::addPath(std::string path)
{
	if(path.size() == 0)
		return;

	wchar_t* ws = KBEngine::char2wchar(path.c_str());
	CString s = ws;
	free(ws);

	HTREEITEM hItemRoot;
	TV_INSERTSTRUCT tcitem;
	tcitem.hParent = TVI_ROOT;
	tcitem.hInsertAfter = TVI_LAST;
	tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tcitem.item.pszText = s.GetBuffer(0);
	tcitem.item.lParam = 0;
	tcitem.item.iImage = 0;
	tcitem.item.iSelectedImage = 1;
	hItemRoot = m_tree.InsertItem(&tcitem);
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

		KBEngine::WatcherObject* wo = watcherPaths.addWatcherFromStream(path, name, id, type, &s);
		addHeader(name);
		addItem(wo);
		addPath(wo->path());
	};
}
