// WatcherWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "WatcherWindow.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "helper/watcher.h"

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
	DDX_Control(pDX, IDC_LIST2, m_statusShow);
}

BOOL CWatcherWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	::SetTimer(m_hWnd, 1, 1500, NULL);

	{
		DWORD dwStyle = m_status.GetExtendedStyle();
		dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
		dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
		//dwStyle |= LVS_EX_ONECLICKACTIVATE;
		m_status.SetExtendedStyle(dwStyle);					//设置扩展风格
	}

	DWORD styles = ::GetWindowLong(m_tree.m_hWnd, GWL_STYLE);
	styles |= TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
	::SetWindowLong(m_tree.m_hWnd, GWL_STYLE, styles);

	{
		DWORD dwStyle = m_statusShow.GetExtendedStyle();
		dwStyle |= LVS_EX_FULLROWSELECT;					//选中某行使整行高亮（只适用与report风格的listctrl）
		dwStyle |= LVS_EX_GRIDLINES;						//网格线（只适用与report风格的listctrl）
		//dwStyle |= LVS_EX_ONECLICKACTIVATE;
		m_statusShow.SetExtendedStyle(dwStyle);				//设置扩展风格
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWatcherWindow::autoWndSize()
{
	CRect rect;
	GetClientRect(&rect);

	m_tree.MoveWindow(2, 3, int((rect.right - 2) * 0.3), rect.bottom - 3, TRUE);
	m_statusShow.MoveWindow(2 + int((rect.right - 2) * 0.3), 3, int((rect.right - 2) * 0.7), rect.bottom - 3, TRUE);
}

BEGIN_MESSAGE_MAP(CWatcherWindow, CDialog)
	ON_WM_TIMER()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CWatcherWindow::OnTvnSelchangedTree1)
END_MESSAGE_MAP()


// CWatcherWindow message handlers

void CWatcherWindow::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
	if(!this->IsWindowVisible())
		return;

	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	dlg->reqQueryWatcher(getCurrSelPath());

	if(m_status.GetItemCount() == 1)
	{
		int nColumnCount = m_status.GetHeaderCtrl()->GetItemCount();   

		if(m_statusShow.GetItemCount() != nColumnCount)
		{
			m_statusShow.InsertColumn(0, L"WatcherName", LVCFMT_LEFT,	150);
			m_statusShow.InsertColumn(1, L"WatcherValue", LVCFMT_LEFT, 1000);

			m_statusShow.DeleteAllItems();

			for (int i=0;i < nColumnCount; i++)
			{
				LVCOLUMN lvcol;
				WCHAR str[256];
				memset(str, 0, 256);
				lvcol.mask = LVCF_TEXT|LVCF_WIDTH;
				lvcol.pszText = str;
				lvcol.cchTextMax = 256;
				lvcol.cx = i;
				m_status.GetColumn(i, &lvcol);
				
				m_statusShow.InsertItem(0, lvcol.pszText);
			}
		}

		for(int ii = 0; ii<m_status.GetHeaderCtrl()->GetItemCount(); ii++)
		{
			CString s = m_status.GetItemText(0, ii);

			LVCOLUMN lvcol;
			WCHAR str[256];
			memset(str, 0, 256);
			lvcol.mask = LVCF_TEXT|LVCF_WIDTH;
			lvcol.pszText = str;
			lvcol.cchTextMax = 256;
			lvcol.cx = ii;
			m_status.GetColumn(ii, &lvcol);

			for(int iix = 0; iix < m_statusShow.GetItemCount(); iix++)
			{
				CString ss = m_statusShow.GetItemText(iix, 0);
				if(ss == lvcol.pszText)
					m_statusShow.SetItemText(iix, 1, s);
			}
		}
	}
	else
	{
		int nColumnCount = 0;

		if(m_status.GetHeaderCtrl())
		{
			nColumnCount = m_status.GetHeaderCtrl()->GetItemCount();     

			if(m_statusShow.GetHeaderCtrl() == NULL || nColumnCount != m_statusShow.GetHeaderCtrl()->GetItemCount() ||
				m_status.GetItemCount() != m_statusShow.GetItemCount())
			{
				m_statusShow.DeleteAllItems();
				if(m_statusShow.GetHeaderCtrl())
				{
					int nColumnCount = m_statusShow.GetHeaderCtrl()->GetItemCount();       
					for (int i=0;i < nColumnCount;i++)
					{
						m_statusShow.DeleteColumn(0);
					}
				}

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

					addHeaderShow(lvcol.pszText);
				}
			}

			for(int i=0; i<m_status.GetItemCount(); i++)
			{
				if(i + 1 > m_statusShow.GetItemCount())
				{
					m_statusShow.InsertItem(0, m_status.GetItemText(i, 0));
				}

				for(int ii = 0; ii<m_status.GetHeaderCtrl()->GetItemCount(); ii++)
				{
					m_statusShow.SetItemText(i, ii, m_status.GetItemText(i, ii));
				}
			}
		}
	}
}

void CWatcherWindow::addHeaderShow(CString name)
{
	int nColumnCount = 0;

	if(m_statusShow.GetHeaderCtrl())
	{
		nColumnCount = m_statusShow.GetHeaderCtrl()->GetItemCount();       
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

			if(name == lvcol.pszText)
				return;
		}
	}

	m_statusShow.InsertColumn(nColumnCount, name, LVCFMT_CENTER,	100);
}

void CWatcherWindow::addHeader(std::string name)
{
	wchar_t* ws = KBEngine::strutil::char2wchar(name.c_str());
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

void CWatcherWindow::addItemShow(KBEngine::WatcherObject* wo)
{
	int idx = 0;
	wchar_t* ws = KBEngine::strutil::char2wchar(wo->str());
	CString s = ws;
	free(ws);

	ws = KBEngine::strutil::char2wchar(wo->name());
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

void CWatcherWindow::addItem(KBEngine::WatcherObject* wo)
{
	int idx = 0;
	wchar_t* ws = KBEngine::strutil::char2wchar(wo->str());
	CString s = ws;
	free(ws);

	ws = KBEngine::strutil::char2wchar(wo->name());
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
	if(clearTree)
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

	m_statusShow.DeleteAllItems();
	if(m_statusShow.GetHeaderCtrl())
	{
		int nColumnCount = m_statusShow.GetHeaderCtrl()->GetItemCount();       
		for (int i=0;i < nColumnCount;i++)
		{
			m_statusShow.DeleteColumn(0);
		}
	}
}

void CWatcherWindow::changePath(std::string path)
{
	clearAllData(false);
}

void CWatcherWindow::addPath(std::string rootpath, std::string path)
{
	if(path.size() == 0)
		return;

	wchar_t* ws = KBEngine::strutil::char2wchar(path.c_str());
	CString s = ws;
	free(ws);

	HTREEITEM hItemRoot = m_tree.GetRootItem();
	hItemRoot = findAndCreatePathItem(rootpath, hItemRoot);

	HTREEITEM item = hItemRoot;
	
	if(item == NULL)
	{
		item = m_tree.GetRootItem();
	}
	else
	{
		item = m_tree.GetChildItem(hItemRoot);
	}

	if(item)
	{
		do
		{
			CString s1 = m_tree.GetItemText(item);
			if(s == s1)
			{
				return;
			}
		}while(item = m_tree.GetNextItem(item, TVGN_NEXT));
	}

	TV_INSERTSTRUCT tcitem;
	tcitem.hParent = hItemRoot;
	tcitem.hInsertAfter = TVI_LAST;
	tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tcitem.item.pszText = s.GetBuffer(0);
	tcitem.item.lParam = 0;
	tcitem.item.iImage = 0;
	tcitem.item.iSelectedImage = 1;
	m_tree.InsertItem(&tcitem);

	m_tree.Expand(hItemRoot, TVE_EXPAND);
	m_tree.Expand(m_tree.GetRootItem(), TVE_EXPAND);
}

HTREEITEM CWatcherWindow::findAndCreatePathItem(std::string path, HTREEITEM rootItem)
{
	if(path == "")
		return NULL;

	std::vector<std::string> vec;
	KBEngine::strutil::kbe_split(path, '/', vec);
	
	wchar_t* ws = KBEngine::strutil::char2wchar(vec[0].c_str());
	CString pathitem = ws;
	free(ws);

	if(vec.size() > 1)
	{
		path.erase(0, vec[0].size() + 1);
		
		HTREEITEM item = rootItem, hasItem = NULL;
		if(item)
		{
			do
			{
				CString s = m_tree.GetItemText(item);
				if(pathitem == s)
				{
					hasItem = item;
					break;
				}
			}while(item = m_tree.GetNextItem(item, TVGN_NEXT));
		}

		if(hasItem == NULL)
		{
			TV_INSERTSTRUCT tcitem;
			tcitem.hParent = rootItem;
			tcitem.hInsertAfter = TVI_LAST;
			tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
			tcitem.item.pszText = pathitem.GetBuffer(0);
			tcitem.item.lParam = 0;
			tcitem.item.iImage = 0;
			tcitem.item.iSelectedImage = 1;
			rootItem = m_tree.InsertItem(&tcitem);
			m_tree.Expand(rootItem, TVE_EXPAND);
		}
		else
		{
			rootItem = hasItem;
			rootItem = m_tree.GetChildItem(hasItem);
			if(rootItem == NULL)
			{
				ws = KBEngine::strutil::char2wchar(vec[1].c_str());
				pathitem = ws;
				free(ws);
				TV_INSERTSTRUCT tcitem;
				tcitem.hParent = rootItem;
				tcitem.hInsertAfter = TVI_LAST;
				tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
				tcitem.item.pszText = pathitem.GetBuffer(0);
				tcitem.item.lParam = 0;
				tcitem.item.iImage = 0;
				tcitem.item.iSelectedImage = 1;
				rootItem = m_tree.InsertItem(&tcitem);
				m_tree.Expand(rootItem, TVE_EXPAND);
			}
		}
		return findAndCreatePathItem(path, rootItem);
	}
	else
	{
		HTREEITEM item = rootItem;

		if(item)
		{
			do
			{
				CString s = m_tree.GetItemText(item);
				if(pathitem == s)
				{
					return item;
				}
			}while(item = m_tree.GetNextItem(item, TVGN_NEXT));
		}
		else
		{
			TV_INSERTSTRUCT tcitem;
			tcitem.hParent = rootItem;
			tcitem.hInsertAfter = TVI_LAST;
			tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
			tcitem.item.pszText = pathitem.GetBuffer(0);
			tcitem.item.lParam = 0;
			tcitem.item.iImage = 0;
			tcitem.item.iSelectedImage = 1;
			rootItem = m_tree.InsertItem(&tcitem);
			m_tree.Expand(rootItem, TVE_EXPAND);
			return rootItem;
		}
	}

	return NULL;
}

void CWatcherWindow::onReceiveWatcherData(KBEngine::MemoryStream& s)
{
	uint8 type = 0;
	s >> type;

	if(type == 0)
	{
		KBEngine::WatcherPaths watcherPaths;

		while(s.length() > 0)
		{
			std::string path;
			s >> path;

			std::string name;
			s >> name;

			KBEngine::WATCHER_ID id = 0;
			s >> id;

			KBEngine::WATCHER_VALUE_TYPE type;
			s >> type;

			KBEngine::WatcherObject* wo = watcherPaths.addWatcherFromStream(path, name, id, type, &s);
			addHeader(name);
			addItem(wo);
		};
	}
	else
	{
		std::string rootpath;
		s >> rootpath;
		if(rootpath == "/")
			rootpath = "";

		while(s.length() > 0)
		{
			std::string path;
			s >> path;
			addPath(rootpath, path);
		}
	}
}

std::string CWatcherWindow::getCurrSelPath()
{
	HTREEITEM item = m_tree.GetSelectedItem();
	if(item == NULL)
	{
		if(m_tree.GetCount() > 0)
			return "root";

		return "";
	}

	CString path;

	do
	{
		CString s;
		if(path.GetLength() == 0)
		{
			path = m_tree.GetItemText(item);
		}
		else
		{
			s = m_tree.GetItemText(item) + "/" + path;
			path = s;
		}
	}while(item = m_tree.GetNextItem(item, TVGN_PARENT));

	char* str = KBEngine::strutil::wchar2char(path.GetBuffer(0));
	std::string ret = str;
	free(str);
	return ret;
}

void CWatcherWindow::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	clearAllData(false);
}
