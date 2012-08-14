
// guiconsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "network/bundle_broadcast.hpp"
#include "server/components.hpp"

#undef DEFINE_IN_INTERFACE
#include "client_lib/client_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "client_lib/client_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"

#include "machine/machine_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "machine/machine_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CguiconsoleDlg dialog




CguiconsoleDlg::CguiconsoleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CguiconsoleDlg::IDD, pParent),
	_componentType(CONSOLE_TYPE),
	_componentID(genUUID64()),
	_dispatcher(),
	_networkInterface(&_dispatcher),
	m_isInit(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CguiconsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_TREE1, m_tree);
}

BEGIN_MESSAGE_MAP(CguiconsoleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_WM_SIZING()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CguiconsoleDlg message handlers

BOOL CguiconsoleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	DebugHelper::initHelper(_componentType);

	::SetTimer(m_hWnd, 1, 100, NULL);
	::SetTimer(m_hWnd, 2, 100, NULL);

	m_isInit = true;

	m_tab.InsertItem(0, _T("DEBUG"), 0); 
	m_debugWnd.Create(IDD_DEBUG, GetDlgItem(IDC_TAB1));

	DWORD styles = ::GetWindowLong(m_tree.m_hWnd, GWL_STYLE);
	styles |= TVS_HASLINES | TVS_LINESATROOT;
	::SetWindowLong(m_tree.m_hWnd, GWL_STYLE, styles);


	autoWndSize();
	updateTree();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CguiconsoleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CguiconsoleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CguiconsoleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CguiconsoleDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);

	switch(nIDEvent)
	{
	case 1:
		break;
	case 2:
		{
			int8 findComponentTypes[] = {BASEAPP_TYPE, CELLAPP_TYPE, BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, LOGINAPP_TYPE, DBMGR_TYPE, UNKNOWN_COMPONENT_TYPE};
			int ifind = 0;

			while(true)
			{
				int8 findComponentType = findComponentTypes[ifind];
				if(findComponentType == UNKNOWN_COMPONENT_TYPE)
				{
					//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
					//	COMPONENT_NAME[findComponentType]);
					
					::KillTimer(m_hWnd, nIDEvent);
					return;
				}

				srand(KBEngine::getSystemTime());
				uint16 nport = KBE_PORT_START + (rand() % 1000);
				Mercury::BundleBroadcast bhandler(_networkInterface, nport);

				if(!bhandler.good())
				{
					KBEngine::sleep(10);
					nport = KBE_PORT_START + (rand() % 1000);
					continue;
				}

				if(bhandler.pCurrPacket() != NULL)
				{
					bhandler.pCurrPacket()->resetPacket();
				}

				bhandler.newMessage(MachineInterface::onFindInterfaceAddr);
				MachineInterface::onFindInterfaceAddrArgs6::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
					_componentType, findComponentType, _networkInterface.intaddr().ip, bhandler.epListen().addr().port);

				if(!bhandler.broadcast())
				{
					::KillTimer(m_hWnd, nIDEvent);
					ERROR_MSG("CguiconsoleDlg::OnTimer: broadcast error!\n");
					::AfxMessageBox(L"初始化错误：不能发送服务器探测包。");
					return;
				}

				MachineInterface::onBroadcastInterfaceArgs8 args;
				if(bhandler.receive(&args, 0))
				{
					if(args.componentType == UNKNOWN_COMPONENT_TYPE)
					{
						//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
						//	COMPONENT_NAME[findComponentType]);
						
						::KillTimer(m_hWnd, nIDEvent);
						return;
					}

					INFO_MSG("CguiconsoleDlg::OnTimer: found %s, addr:%s:%u\n",
						COMPONENT_NAME[args.componentType], inet_ntoa((struct in_addr&)args.intaddr), ntohs(args.intaddr));

					Components::getSingleton().addComponent(args.uid, args.username.c_str(), 
						(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.intaddr, args.intport, args.extaddr, args.extport);
					
					updateTree();

					// 防止接收到的数据不是想要的数据
					if(findComponentType == args.componentType)
					{
						ifind++;
					}
					else
					{
						ERROR_MSG("CguiconsoleDlg::OnTimer: %s not found. receive data is error!\n", COMPONENT_NAME[findComponentType]);
					}
				}
				else
				{
					::KillTimer(m_hWnd, nIDEvent);
					ERROR_MSG("CguiconsoleDlg::OnTimer: receive error!\n");
					//::AfxMessageBox(L"初始化错误：未找到服务器。");
					updateTree();
					return;
				}
			}

			::KillTimer(m_hWnd, nIDEvent);
		}
		break;
	default:
		break;
	};

		
}

void CguiconsoleDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	autoWndSize();
	// TODO: Add your message handler code here
}

void CguiconsoleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	autoWndSize();
}

void CguiconsoleDlg::updateTree()
{
	if(!m_isInit)
		return;

	m_tree.DeleteAllItems();

	Components::COMPONENTS cts0 = Components::getSingleton().getComponents(BASEAPP_TYPE);
	Components::COMPONENTS cts1 = Components::getSingleton().getComponents(CELLAPP_TYPE);
	Components::COMPONENTS cts2 = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::COMPONENTS cts3 = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Components::COMPONENTS cts4 = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::COMPONENTS cts5 = Components::getSingleton().getComponents(LOGINAPP_TYPE);
	Components::COMPONENTS cts;
	
	if(cts0.size() > 0)
		cts.insert(cts.begin(), cts0.begin(), cts0.end());
	if(cts1.size() > 0)
		cts.insert(cts.begin(), cts1.begin(), cts1.end());
	if(cts2.size() > 0)
		cts.insert(cts.begin(), cts2.begin(), cts2.end());
	if(cts3.size() > 0)
		cts.insert(cts.begin(), cts3.begin(), cts3.end());
	if(cts4.size() > 0)
		cts.insert(cts.begin(), cts4.begin(), cts4.end());
	if(cts5.size() > 0)
		cts.insert(cts.begin(), cts5.begin(), cts5.end());

	HTREEITEM hItemRoot;
	TV_INSERTSTRUCT tcitem;
	tcitem.hParent = TVI_ROOT;
	tcitem.hInsertAfter = TVI_LAST;
	tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tcitem.item.pszText = L"servers";
	tcitem.item.lParam = 0;
	tcitem.item.iImage = 0;
	tcitem.item.iSelectedImage = 1;
	hItemRoot = m_tree.InsertItem(&tcitem);

	if(cts.size() == 0)
	{
		tcitem.hParent = hItemRoot;
		tcitem.hInsertAfter = TVI_LAST;
		tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		tcitem.item.pszText = L"server not found!";
		tcitem.item.lParam = 0;
		tcitem.item.iImage = 0;
		tcitem.item.iSelectedImage = 1;
		m_tree.InsertItem(&tcitem);
	}
	else
	{
		Components::COMPONENTS::iterator iter = cts.begin();
		for(; iter != cts.end(); iter++)
		{
			Components::ComponentInfos& cinfos = (*iter);

			HTREEITEM item = m_tree.GetChildItem(hItemRoot), hasUIDItem = NULL;

			if(item)
			{
				do
				{
					CString s = m_tree.GetItemText(item);
					CString s1;
					s1.Format(L"uid[%u]", cinfos.uid);

					if(s1 == s)
					{
						hasUIDItem = item;
						break;
					}
				}while(item = m_tree.GetNextItem(item, TVGN_NEXT));
			}

			if(hasUIDItem == NULL)
			{
				CString s;
				s.Format(L"uid[%u]", cinfos.uid);
				tcitem.hParent = hItemRoot;
				tcitem.hInsertAfter = TVI_LAST;
				tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
				tcitem.item.pszText = s.GetBuffer(0);
				tcitem.item.lParam = 0;
				tcitem.item.iImage = 0;
				tcitem.item.iSelectedImage = 1;
				hasUIDItem = m_tree.InsertItem(&tcitem);
			}
			
			char sbuf[1024];
			kbe_snprintf(sbuf, 1024, "%s[%s]", COMPONENT_NAME[cinfos.componentType], cinfos.pIntAddr->c_str());
			wchar_t* wbuf = KBEngine::char2wchar(sbuf);
			tcitem.hParent = hasUIDItem;
			tcitem.hInsertAfter = TVI_LAST;
			tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
			tcitem.item.pszText = wbuf;
			tcitem.item.lParam = 0;
			tcitem.item.iImage = 0;
			tcitem.item.iSelectedImage = 1;
			m_tree.InsertItem(&tcitem);
			m_tree.Expand(hasUIDItem, TVE_EXPAND);
			free(wbuf);
		}
	}

	m_tree.Expand(hItemRoot, TVE_EXPAND);
}

void CguiconsoleDlg::autoWndSize()
{
	if(!m_isInit)
		return;

	CRect rect1;
	this->GetClientRect(&rect1);
	rect1.left += int(rect1.right * 0.25);
	m_tab.MoveWindow(rect1);
	
	CRect rect;
	m_tab.GetClientRect(&rect);

	CRect rect2;
	this->GetClientRect(&rect2);
	rect2.right = int(rect2.right * 0.25);
	m_tree.MoveWindow(rect2);

	rect.top += 25;
	rect.bottom -= 5;
	rect.left += 5;
	rect.right -= 5;
	m_debugWnd.MoveWindow(&rect);
	m_debugWnd.autoWndSize();
	m_debugWnd.ShowWindow(SW_SHOW);
}
