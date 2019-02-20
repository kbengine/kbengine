
// guiconsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "StartServerWindow.h"
#include "network/bundle_broadcast.h"
#include "network/message_handler.h"
#include "server/components.h"
#include "helper/console_helper.h"
#include "xml/xml.h"

#undef DEFINE_IN_INTERFACE
#include "client_lib/client_interface.h"
#define DEFINE_IN_INTERFACE
#include "client_lib/client_interface.h"

#undef DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.h"
#define DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.h"

#undef DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.h"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.h"

#undef DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.h"
#define DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.h"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.h"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.h"

#undef DEFINE_IN_INTERFACE
#include "machine/machine_interface.h"
#define DEFINE_IN_INTERFACE
#include "machine/machine_interface.h"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.h"

#undef DEFINE_IN_INTERFACE
#include "tools/logger/logger_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/logger/logger_interface.h"

#undef DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.h"

#undef DEFINE_IN_INTERFACE
#include "tools/interfaces/interfaces_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/interfaces/interfaces_interface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace KBEngine{
namespace ConsoleInterface{
	Network::MessageHandlers messageHandlers("ConsoleInterface");
}
}

BOOL g_sendData = FALSE;

class ConsoleExecCommandCBMessageHandlerEx : public KBEngine::ConsoleInterface::ConsoleExecCommandCBMessageHandler
{
public:
	virtual void handle(Network::Channel* pChannel, MemoryStream& s)
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		std::string strarg;	
		s.readBlob(strarg);
		dlg->onExecScriptCommandCB(pChannel, strarg);
	};
};

class ConsoleLogMessageHandlerEx : public KBEngine::ConsoleInterface::ConsoleLogMessageHandler
{
public:
	virtual void handle(Network::Channel* pChannel, MemoryStream& s)
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		std::string str;
		s.readBlob(str);
		dlg->onReceiveRemoteLog(str);
	};
};

class ConsoleWatcherCBMessageHandlerEx : public KBEngine::ConsoleInterface::ConsoleWatcherCBMessageHandler
{
public:
	virtual void handle(Network::Channel* pChannel, MemoryStream& s)
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		dlg->onReceiveWatcherData(s);
	};
};

class ConsoleProfileHandlerEx : public KBEngine::ConsoleInterface::ConsoleProfileHandler
{
public:
	virtual void handle(Network::Channel* pChannel, MemoryStream& s)
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		dlg->onReceiveProfileData(s);
	};
};

class FindServersTask : public thread::TPTask
{
public:
	std::vector<COMPONENT_TYPE> findComponentTypes;

	FindServersTask():
	thread::TPTask(),
	findComponentTypes()
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		dlg->clearTree();
	}

	FindServersTask(COMPONENT_TYPE findComponentType):
	thread::TPTask(),
	findComponentTypes()
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		dlg->clearTree();

		findComponentTypes.push_back(findComponentType);
	}

	virtual ~FindServersTask()
	{
	}

	virtual bool process()
	{
		//COMPONENT_TYPE findComponentTypes[] = {LOGGER_TYPE, BASEAPP_TYPE, CELLAPP_TYPE, BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, LOGINAPP_TYPE, DBMGR_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};
		int ifind = 0;

		if(g_isDestroyed)
			return false;

		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);

		while(true)
		{
			if(ifind >= (int)findComponentTypes.size() || g_isDestroyed)
			{
				//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
				//	COMPONENT_NAME_EX(findComponentType));
				return false;
			}

			COMPONENT_TYPE findComponentType = findComponentTypes[ifind];

			dlg->updateFindTreeStatus();
			srand(KBEngine::getSystemTime());
			uint16 nport = KBE_PORT_START + (rand() % 1000);
			Network::BundleBroadcast bhandler(dlg->networkInterface(), nport);

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
			MachineInterface::onFindInterfaceAddrArgs7::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
				dlg->componentType(), dlg->componentID(), (COMPONENT_TYPE)findComponentType, dlg->networkInterface().intTcpAddr().ip,
				bhandler.epListen().addr().port);

			if(!bhandler.broadcast())
			{
				ERROR_MSG("CguiconsoleDlg::OnTimer: broadcast error!\n");
				::AfxMessageBox(L"初始化错误：不能发送服务器探测包。");
				return false;
			}

			MachineInterface::onBroadcastInterfaceArgs25 args;
			int32 timeout = 1000000;

RESTART_RECV:
			if(bhandler.receive(&args, 0, timeout))
			{
				bool isContinue = false;

				do
				{
					if(g_isDestroyed)
						return false;

					if(isContinue)
					{
						try
						{
							args.createFromStream(*bhandler.pCurrPacket());
						}catch(MemoryStreamException &)
						{
							break;
						}
					}

					if(args.componentType == UNKNOWN_COMPONENT_TYPE)
					{
						//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
						//	COMPONENT_NAME_EX(findComponentType));
						//ifind++;
						isContinue = true;
						continue;
					}

					INFO_MSG(fmt::format("CguiconsoleDlg::OnTimer: found {}, addr:{}:{}\n",
						COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType), inet_ntoa((struct in_addr&)args.intaddr), ntohs(args.intport)));

					Components::getSingleton().addComponent(args.uid, args.username.c_str(), 
						(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.globalorderid, args.grouporderid, args.gus,
						args.intaddr, args.intport, args.extaddr, args.extport, args.extaddrEx, args.pid, args.cpu, args.mem, args.usedmem, 
						args.extradata, args.extradata1, args.extradata2, args.extradata3);
					
					isContinue = true;
				}while(bhandler.pCurrPacket()->length() > 0);

				// 防止接收到的数据不是想要的数据
				if(findComponentType == args.componentType)
				{
					//ifind++;
					if(g_isDestroyed)
						return false;

					dlg->updateTree();
				}
				else
				{
					ERROR_MSG(fmt::format("CguiconsoleDlg::OnTimer: {} not found. receive data error!\n",
						COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));
				}

				//timeout = 10000;
				goto RESTART_RECV;
			}
			else
			{
				ifind++;
				continue;
			}
		}

		return false;
	}

	virtual thread::TPTask::TPTaskState presentMainThread()
	{ 
		if(!g_isDestroyed)
		{
			CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
			dlg->updateTree();
		}

		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
};


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
	m_debugWnd(),
	m_logWnd(),
	m_statusWnd(),
	m_profileWnd(),
	m_watcherWnd(),
	m_spaceViewWnd(),
	m_graphsWindow(),
	m_isInit(false),
	m_historyCommand(),
	m_historyCommandIndex(0),
	m_isUsingHistroy(false),
	threadPool_()
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
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CguiconsoleDlg::OnNMRClickTree1)
	ON_COMMAND(ID_32772, &CguiconsoleDlg::OnMenu_Update)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CguiconsoleDlg::OnTcnSelchangeTab1)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, &CguiconsoleDlg::OnNMClickTree1)
	ON_COMMAND(ID_32771, &CguiconsoleDlg::OnConnectRemoteMachine)
	ON_COMMAND(ID_HELP_ABOUT, &CguiconsoleDlg::OnHelpAbout)
	ON_COMMAND(ID_BUTTON32784, &CguiconsoleDlg::OnToolBar_Find)
	ON_COMMAND(ID_BUTTON32780, &CguiconsoleDlg::OnToolBar_StartServer)
	ON_COMMAND(ID_BUTTON32783, &CguiconsoleDlg::OnToolBar_StopServer)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
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
	
	if(!m_ToolBar.CreateEx(this, TBSTYLE_FLAT ,  WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS,
    CRect(4,4,0,0))
		||!m_ToolBar.LoadToolBar(IDR_TOOLBAR1))
	{
		return FALSE;
	}

	m_ToolBar.ShowWindow(SW_SHOW);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	
	KBEngine::Network::MessageHandlers::pMainMessageHandlers = &KBEngine::ConsoleInterface::messageHandlers;

	// TODO: Add extra initialization here
	_dispatcher.breakProcessing(false);

	::SetTimer(m_hWnd, 1, 10, NULL);
	::SetTimer(m_hWnd, 2, 100, NULL);
	::SetTimer(m_hWnd, 3, 1000 * 20, NULL);

	m_isInit = true;
	
	m_tab.InsertItem(0, _T("Status"), 0); 
	m_statusWnd.Create(IDD_STATUS, GetDlgItem(IDC_TAB1));
	
	m_tab.InsertItem(1, _T("Debug"), 0); 
	m_debugWnd.Create(IDD_DEBUG, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(2, _T("Log"), 0); 
	m_logWnd.Create(IDD_LOG, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(3, _T("Profile"), 0); 
	m_profileWnd.Create(IDD_PROFILE, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(4, _T("Watcher"), 0); 
	m_watcherWnd.Create(IDD_WATCHER, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(5, _T("SpaceView"), 0); 
	m_spaceViewWnd.Create(IDD_SPACEVIEW, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(6, _T("Graphs"), 0); 
	m_graphsWindow.Create(IDD_GRAPHS, GetDlgItem(IDC_TAB1));
	
	DWORD styles = ::GetWindowLong(m_tree.m_hWnd, GWL_STYLE);
	styles |= TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
	::SetWindowLong(m_tree.m_hWnd, GWL_STYLE, styles);

	loadHistory();
	autoWndSize();
	updateTree();

	KBEngine::ConsoleInterface::messageHandlers.add("Console::onExecScriptCommandCB", new KBEngine::ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1, NETWORK_VARIABLE_MESSAGE, 
		new ConsoleExecCommandCBMessageHandlerEx);

	KBEngine::ConsoleInterface::messageHandlers.add("Console::onReceiveRemoteLog", new KBEngine::ConsoleInterface::ConsoleLogMessageHandlerArgsStream, NETWORK_VARIABLE_MESSAGE, 
		new ConsoleLogMessageHandlerEx);

	KBEngine::ConsoleInterface::messageHandlers.add("Console::onReceiveWatcherData", new KBEngine::ConsoleInterface::ConsoleWatcherCBHandlerMessageArgsStream, NETWORK_VARIABLE_MESSAGE, 
		new ConsoleWatcherCBMessageHandlerEx);
	
	KBEngine::ConsoleInterface::messageHandlers.add("Console::onReceiveProfileData", new KBEngine::ConsoleInterface::ConsoleProfileHandlerArgsStream, NETWORK_VARIABLE_MESSAGE, 
		new ConsoleProfileHandlerEx);

	KBEngine::Network::Bundle::ObjPool().pMutex(new KBEngine::thread::ThreadMutex());
	KBEngine::Network::TCPPacket::ObjPool().pMutex(new KBEngine::thread::ThreadMutex());
	KBEngine::Network::UDPPacket::ObjPool().pMutex(new KBEngine::thread::ThreadMutex());
	KBEngine::MemoryStream::ObjPool().pMutex(new KBEngine::thread::ThreadMutex());
	threadPool_.createThreadPool(1, 1, 16);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CguiconsoleDlg::onReceiveRemoteLog(std::string str)
{
	m_logWnd.onReceiveRemoteLog(str);
}

void CguiconsoleDlg::historyCommandCheck()
{
	if(m_historyCommand.size() > 50)
		m_historyCommand.pop_front();

	if(m_historyCommandIndex < 0)
		m_historyCommandIndex = m_historyCommand.size() - 1;

	if(m_historyCommandIndex > (int)m_historyCommand.size() - 1)
		m_historyCommandIndex = 0; 
}

CString CguiconsoleDlg::getHistoryCommand(bool isNextCommand)
{
	if(m_isUsingHistroy)
	{
		if(isNextCommand)
			m_historyCommandIndex++;
		else
			m_historyCommandIndex--;
	}

	m_isUsingHistroy = true;
	historyCommandCheck();

	if(m_historyCommand.size() == 0)
		return L"";
	return m_historyCommand[m_historyCommandIndex];
}

HTREEITEM CguiconsoleDlg::hasCheckApp(COMPONENT_TYPE type)
{
	HTREEITEM rootitem = m_tree.GetRootItem();
	if(rootitem == NULL)
		return NULL;
	
	rootitem = m_tree.GetChildItem(rootitem);
	if(rootitem == NULL)
		return NULL;

	do
	{
		HTREEITEM childItem = m_tree.GetChildItem(rootitem);
		while(NULL != childItem)
		{
			COMPONENT_TYPE ctype = getTreeItemComponent(childItem);

			if(ctype == type)
			{
				if(m_tree.GetCheck(childItem))
					return childItem;
			}

			childItem = m_tree.GetNextItem(childItem, TVGN_NEXT);
		}
	}while((rootitem = m_tree.GetNextItem(rootitem, TVGN_NEXT)) != NULL);

	return NULL;
}

void CguiconsoleDlg::onReceiveProfileData(MemoryStream& s)
{
	KBEngine::int8 type;
	s >> type;

	m_profileWnd.onReceiveData(type, s);
}

void CguiconsoleDlg::commitPythonCommand(CString strCommand)
{
	strCommand.Replace(L"\r", L"");
	if(strCommand.GetLength() <= 0)
		return;

	if(getTreeItemComponent(m_tree.GetSelectedItem()) != CELLAPP_TYPE 
		&& getTreeItemComponent(m_tree.GetSelectedItem()) != BASEAPP_TYPE
		&& getTreeItemComponent(m_tree.GetSelectedItem()) != BOTS_TYPE)
	{
		::AfxMessageBox(L"Component can not debug!");
		return;
	}
	
	m_isUsingHistroy = false;

	m_historyCommand.push_back(strCommand);
	historyCommandCheck();
	m_historyCommandIndex = m_historyCommand.size() - 1;

	CString strCommand1 = strCommand;

	/*
	// 对普通的输入加入print 让服务器回显信息
    if((strCommand.Find(L"=")) == -1 &&
		(strCommand.Find(L"print(")) == -1 &&
		(strCommand.Find(L"import ")) == -1 &&
		(strCommand.Find(L"class ")) == -1 &&
		(strCommand.Find(L"def ")) == -1 &&
		(strCommand.Find(L"del ")) == -1 &&
		(strCommand.Find(L"if ")) == -1 &&
		(strCommand.Find(L"for ")) == -1 &&
		(strCommand.Find(L"while ")) == -1
		)
        strCommand = L"print(" + strCommand + L")";
	*/

	std::wstring incmd = strCommand.GetBuffer(0);
	std::string outcmd;
	strutil::wchar2utf8(incmd, outcmd);

	
	Network::Channel* pChannel = _networkInterface.findChannel(this->getTreeItemAddr(m_tree.GetSelectedItem()));
	if(pChannel)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		if(getTreeItemComponent(m_tree.GetSelectedItem()) == BASEAPP_TYPE)
			(*pBundle).newMessage(BaseappInterface::onExecScriptCommand);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == CELLAPP_TYPE)
			(*pBundle).newMessage(CellappInterface::onExecScriptCommand);
		else
			(*pBundle).newMessage(BotsInterface::onExecScriptCommand);

		ArraySize size = outcmd.size();
		(*pBundle) << size;
		(*pBundle).append(outcmd.data(), size);
		pChannel->send(pBundle);

		CString str1, str2;
		m_debugWnd.displaybufferWnd()->GetWindowText(str2);
		wchar_t sign[10] = {L">>>"};
		str1.Append(sign);
		str1.Append(strCommand1);
		g_sendData = TRUE;
		str2 += str1;
		m_debugWnd.displaybufferWnd()->SetWindowTextW(str2);

		saveHistory();
	}
}

void CguiconsoleDlg::saveHistory()
{
    //创建一个XML的文档对象。
    TiXmlDocument *pDocument = new TiXmlDocument();

	int i = 0;
	std::deque<CString>::iterator iter = m_historyCommand.begin();
	TiXmlElement *rootElement = new TiXmlElement("root");
	pDocument->LinkEndChild(rootElement);

	for(; iter != m_historyCommand.end(); iter++)
	{
		char key[256] = {0};
		kbe_snprintf(key, 256, "item%d", i++);
		TiXmlElement *rootElementChild = new TiXmlElement(key);
		rootElement->LinkEndChild(rootElementChild);

		std::wstring strCommand = (*iter);
		std::string str;
		
		strutil::wchar2utf8(strCommand, str);
		TiXmlText *content = new TiXmlText(str.data());
		rootElementChild->LinkEndChild(content);
	}

    CString appPath = GetAppPath();
    CString fullPath = appPath + L"\\histroycommands.xml";

	char fname[4096] = {0};

	int len = WideCharToMultiByte(CP_ACP, 0, fullPath, fullPath.GetLength(), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP,0, fullPath, fullPath.GetLength(), fname, len, NULL, NULL);
	fname[len] = '\0';

	pDocument->SaveFile(fname);
}

void CguiconsoleDlg::loadHistory()
{
    CString appPath = GetAppPath();
    CString fullPath = appPath + L"\\histroycommands.xml";

	char fname[4096] = {0};

	int len = WideCharToMultiByte(CP_ACP, 0, fullPath, fullPath.GetLength(), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP,0, fullPath, fullPath.GetLength(), fname, len, NULL, NULL);
	fname[len + 1] = '\0';

	TiXmlDocument *pDocument = new TiXmlDocument(fname);
	if(pDocument == NULL || !pDocument->LoadFile(TIXML_ENCODING_UTF8))
		return;

	TiXmlElement *rootElement = pDocument->RootElement();
	TiXmlNode* node = rootElement->FirstChild();
	if(node)
	{
		do																				
		{	
			if(node->FirstChild() != NULL)
			{
				std::string c = node->FirstChild()->Value();
				if(c.size() > 0)
				{
					std::wstring strCommand;
					strutil::utf82wchar(c, strCommand);
					CString sstrCommand = strCommand.data();
					m_historyCommand.push_back(sstrCommand);
				}
			}
		}while((node = node->NextSibling()));												
	}

	pDocument->Clear();
	delete pDocument;
}	

BOOL CguiconsoleDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)   
	{
		if(pMsg-> wParam == 0x0d)   
		{
			return false;
		} 
	}

	return CDialog::PreTranslateMessage(pMsg);
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

void CguiconsoleDlg::addThreadTask(thread::TPTask* tptask)
{
	threadPool_.addTask(tptask);
}

void CguiconsoleDlg::updateFindTreeStatus()
{
	static int count = 0;
	if(count++ >= 6)
	{
		count = 0;
	}

	CString s = L"find server";

	for(int i=0; i<count; i++)
	{
		s += L".";
	}

	this->SetWindowTextW(s.GetBuffer(0));

	/*
	m_tree.DeleteAllItems();
	m_statusWnd.m_statusList.DeleteAllItems();

	HTREEITEM hItemRoot;
	TV_INSERTSTRUCT tcitem;
	tcitem.hParent = TVI_ROOT;
	tcitem.hInsertAfter = TVI_LAST;
	tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tcitem.item.pszText = L"servergroups";
	tcitem.item.lParam = 0;
	tcitem.item.iImage = 0;
	tcitem.item.iSelectedImage = 1;
	hItemRoot = m_tree.InsertItem(&tcitem);

	tcitem.hParent = hItemRoot;
	tcitem.hInsertAfter = TVI_LAST;
	tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	CString s = L"find server";

	for(int i=0; i<count; i++)
	{
		s += L".";
	}

	tcitem.item.pszText = s.GetBuffer(0);
	tcitem.item.lParam = 0;
	tcitem.item.iImage = 0;
	tcitem.item.iSelectedImage = 1;
	m_tree.InsertItem(&tcitem);
	m_tree.Expand(hItemRoot, TVE_EXPAND);
	*/
}

void CguiconsoleDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);

	if(g_sendData)
	{
		g_sendData = !g_sendData;
		m_debugWnd.sendbufferWnd()->SetWindowTextW(L"");
	}

	switch(nIDEvent)
	{
	case 1:
		threadPool_.onMainThreadTick();
		_dispatcher.processOnce(false);
		_networkInterface.processChannels(&KBEngine::ConsoleInterface::messageHandlers);
		break;
	case 2:
		{
			threadPool_.addTask(new FindServersTask(LOGGER_TYPE));
			threadPool_.addTask(new FindServersTask(BASEAPP_TYPE));
			threadPool_.addTask(new FindServersTask(CELLAPP_TYPE));
			threadPool_.addTask(new FindServersTask(BASEAPPMGR_TYPE));
			threadPool_.addTask(new FindServersTask(CELLAPPMGR_TYPE));
			threadPool_.addTask(new FindServersTask(LOGINAPP_TYPE));
			threadPool_.addTask(new FindServersTask(DBMGR_TYPE));
			threadPool_.addTask(new FindServersTask(BOTS_TYPE));
			::KillTimer(m_hWnd, nIDEvent);
		}
		break;
	case 3:
		{
			const Network::NetworkInterface::ChannelMap& channels = _networkInterface.channels();
			Network::NetworkInterface::ChannelMap::const_iterator iter = channels.begin();
			for(; iter != channels.end(); iter++)
			{
				Network::Channel* pChannel = const_cast<KBEngine::Network::Channel*>(iter->second);
				Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

				if(pChannel->proxyID() != BOTS_TYPE)
				{
					COMMON_NETWORK_MESSAGE((KBEngine::COMPONENT_TYPE)pChannel->proxyID(), (*pBundle), onAppActiveTick);
				}
				else
				{
					(*pBundle).newMessage(BotsInterface::onAppActiveTick);
				}

				(*pBundle) << _componentType;
				(*pBundle) << _componentID;
				pChannel->send(pBundle);

				pChannel->updateLastReceivedTime();
			}
		}
		break;
	default:
		break;
	};	
}

void CguiconsoleDlg::onExecScriptCommandCB(Network::Channel* pChannel, std::string& command)
{
	DEBUG_MSG(fmt::format("CguiconsoleDlg::onExecScriptCommandCB: {}\n", command.c_str()));

	std::wstring wcmd;
	strutil::utf82wchar(command, wcmd);

	CString str;
	CEdit* lpEdit = (CEdit*)m_debugWnd.displaybufferWnd();
	lpEdit->GetWindowText(str);

	if(str.GetLength() > 0)
		str += L"\r\n";
	else
		str += L"";

	for(unsigned int i=0; i<wcmd.size(); i++)
	{
		wchar_t c = wcmd.c_str()[i];
		switch(c)
		{
		case 10:
			str += L"\r\n";
			break;
		case 32:
			str += c;
			break;
		case 34:
			str += L"\t";
			break;
		default:
			str += c;
		}
	}

	str += L"\r\n";
	lpEdit->SetWindowText(str.GetBuffer(0));
	int nline = lpEdit->GetLineCount();   
	lpEdit->LineScroll(nline - 1);
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

bool CguiconsoleDlg::hasTreeComponent(Components::ComponentInfos& cinfos)
{
	HTREEITEM hItemRoot = m_tree.GetRootItem();
	if(hItemRoot == NULL)
		return false;

	HTREEITEM item = m_tree.GetChildItem(hItemRoot), hasUIDItem = NULL;

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

	if(hasUIDItem == NULL)
		return false;

	item = m_tree.GetChildItem(hasUIDItem);

	char sbuf[1024];
	kbe_snprintf(sbuf, 1024, "%s[%s]", COMPONENT_NAME_EX(cinfos.componentType), cinfos.pIntAddr->c_str());
	wchar_t* wbuf = KBEngine::strutil::char2wchar(sbuf);
	CString s1 = wbuf;
	free(wbuf);

	do
	{
		CString s = m_tree.GetItemText(item);

		if(s1 == s)
		{
			return true;
		}
	}while(item = m_tree.GetNextItem(item, TVGN_NEXT));

	return false;
}

void CguiconsoleDlg::updateTree()
{
	if(!m_isInit)
		return;

	Components::COMPONENTS& cts0 = Components::getSingleton().getComponents(BASEAPP_TYPE);
	Components::COMPONENTS& cts1 = Components::getSingleton().getComponents(CELLAPP_TYPE);
	Components::COMPONENTS& cts2 = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::COMPONENTS& cts3 = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Components::COMPONENTS& cts4 = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::COMPONENTS& cts5 = Components::getSingleton().getComponents(LOGINAPP_TYPE);
	Components::COMPONENTS& cts6 = Components::getSingleton().getComponents(LOGGER_TYPE);
	Components::COMPONENTS& cts7 = Components::getSingleton().getComponents(BOTS_TYPE);
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
	if(cts6.size() > 0)
		cts.insert(cts.begin(), cts6.begin(), cts6.end());
	if(cts7.size() > 0)
		cts.insert(cts.begin(), cts7.begin(), cts7.end());

	HTREEITEM hItemRoot;
	TV_INSERTSTRUCT tcitem;

	hItemRoot = m_tree.GetRootItem();
	if(hItemRoot == NULL)
	{
		tcitem.hParent = TVI_ROOT;
		tcitem.hInsertAfter = TVI_LAST;
		tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		tcitem.item.pszText = L"servergroups";
		tcitem.item.lParam = 0;
		tcitem.item.iImage = 0;
		tcitem.item.iSelectedImage = 1;
		hItemRoot = m_tree.InsertItem(&tcitem);
	}

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
		
		if(this->hasTreeComponent(cinfos))
		{
			continue;
		}
		
		char sbuf[1024];
		kbe_snprintf(sbuf, 1024, "%s[%s]", COMPONENT_NAME_EX(cinfos.componentType), cinfos.pIntAddr->c_str());
		wchar_t* wbuf = KBEngine::strutil::char2wchar(sbuf);
		tcitem.hParent = hasUIDItem;
		tcitem.hInsertAfter = TVI_LAST;
		tcitem.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		tcitem.item.pszText = wbuf;
		tcitem.item.lParam = 0;
		tcitem.item.iImage = 0;
		tcitem.item.iSelectedImage = 1;
		HTREEITEM insertItem = m_tree.InsertItem(&tcitem);
		m_tree.SetItemData(insertItem, (DWORD)&cinfos.cid);
		m_tree.Expand(hasUIDItem, TVE_EXPAND);
		free(wbuf);

		m_statusWnd.addApp(cinfos);
	}

	m_tree.Expand(hItemRoot, TVE_EXPAND);
	
	ListSortData *tmpp = new ListSortData;
	tmpp->listctrl = &m_statusWnd.m_statusList;
	tmpp->isub = 0;
	tmpp->seq = 0;
	m_statusWnd.m_statusList.SortItems(CompareFunc,(LPARAM)tmpp);
	delete tmpp;
}

void CguiconsoleDlg::autoWndSize()
{
	if(!m_isInit)
		return;

	CRect rect1;
	m_ToolBar.GetClientRect(&rect1);
	g_diffHeight = rect1.bottom + 8;
	
	this->GetClientRect(&rect1);
	rect1.left += int(rect1.right * 0.25);
	rect1.top += g_diffHeight;
	m_tab.MoveWindow(rect1);
	
	CRect rect;
	m_tab.GetClientRect(&rect);

	CRect rect2;
	this->GetClientRect(&rect2);
	rect2.right = int(rect2.right * 0.25);
	rect2.top += g_diffHeight;
	m_tree.MoveWindow(rect2);

	rect.top += 25;
	rect.bottom -= 5;
	rect.left += 5;
	rect.right -= 5;

	m_statusWnd.MoveWindow(&rect);
	m_statusWnd.autoWndSize();

	m_logWnd.MoveWindow(&rect);
	m_logWnd.autoWndSize();

	m_debugWnd.MoveWindow(&rect);
	m_debugWnd.autoWndSize();

	m_profileWnd.MoveWindow(&rect);
	m_profileWnd.autoWndSize();

	m_watcherWnd.MoveWindow(&rect);
	m_watcherWnd.autoWndSize();

	m_spaceViewWnd.MoveWindow(&rect);
	m_spaceViewWnd.autoWndSize();

	m_graphsWindow.MoveWindow(&rect);
	m_graphsWindow.autoWndSize();

	autoShowWindow();
}

void CguiconsoleDlg::OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CMenu menu;
    menu.LoadMenu(IDR_POPMENU);
    CMenu* pPopup = menu.GetSubMenu(0);
	
	CPoint point;
	GetCursorPos(&point); //鼠标位置
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CguiconsoleDlg::reqQueryWatcher(std::string paths)
{
	COMPONENT_TYPE debugComponentType = getTreeItemComponent(m_tree.GetSelectedItem());
	Network::Address addr = getTreeItemAddr(m_tree.GetSelectedItem());

	Network::Channel* pChannel = _networkInterface.findChannel(addr);

	if(pChannel)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

		if(debugComponentType == BOTS_TYPE)
		{
			(*pBundle).newMessage(BotsInterface::queryWatcher);
		}
		else
		{
			COMMON_NETWORK_MESSAGE(debugComponentType, (*pBundle), queryWatcher);
		}

		(*pBundle) << paths;
		pChannel->send(pBundle);
	}
}

COMPONENT_TYPE CguiconsoleDlg::getTreeItemComponent(HTREEITEM hItem)
{
	if(hItem == NULL)
		return UNKNOWN_COMPONENT_TYPE;

	CString s = m_tree.GetItemText(hItem);
	int fi_cellappmgr = s.Find(L"cellappmgr", 0);
	int fi_baseappmgr = s.Find(L"baseappmgr", 0);
	int fi_cellapp = s.Find(L"cellapp", 0);
	int fi_baseapp = s.Find(L"baseapp", 0);
	if(fi_cellappmgr >= 0)
		fi_cellapp = -1;
	if(fi_baseappmgr >= 0)
		fi_baseapp = -1;
	int fi_loginapp = s.Find(L"loginapp", 0);
	int fi_dbmgr = s.Find(L"dbmgr", 0);
	int fi_logger = s.Find(L"logger", 0);
	int fi_bots = s.Find(L"bots", 0);

	if(fi_cellapp  < 0 &&
		fi_baseapp < 0 &&
		fi_cellappmgr < 0 &&
		fi_baseappmgr < 0 &&
		fi_loginapp < 0 &&
		fi_logger < 0 &&
		fi_dbmgr < 0 &&
		fi_bots < 0)
	{
		return UNKNOWN_COMPONENT_TYPE;
	}

	if(fi_cellapp >= 0)
	{
		return CELLAPP_TYPE;
	}
	else if(fi_baseapp >= 0)
	{
		return BASEAPP_TYPE;
	}
	else if(fi_cellappmgr >= 0)
	{
		return CELLAPPMGR_TYPE;
	}
	else if(fi_baseappmgr >= 0)
	{
		return BASEAPPMGR_TYPE;
	}
	else if(fi_loginapp >= 0)
	{
		return LOGINAPP_TYPE;
	}
	else if(fi_dbmgr >= 0)
	{
		return DBMGR_TYPE;
	}
	else if(fi_logger >= 0)
	{
		return LOGGER_TYPE;
	}
	else if(fi_bots >= 0)
	{
		return BOTS_TYPE;
	}

	return UNKNOWN_COMPONENT_TYPE;
}

int32 CguiconsoleDlg::getSelTreeItemUID()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if(hItem == NULL)
		return 0;

	CString s = m_tree.GetItemText(hItem);
	if(s.Find(L"uid[") >= 0)
	{
		s.Replace(L"uid[", L"");
		s.Replace(L"]", L"");
	}
	else
	{
		hItem = m_tree.GetParentItem(hItem);
		if(hItem == NULL)
			return 0;

		s = m_tree.GetItemText(hItem);
		if(s.Find(L"uid[") >= 0)
		{
			s.Replace(L"uid[", L"");
			s.Replace(L"]", L"");
		}
		else
		{
			s = L"";
		}
	}

	if(s.GetLength() == 0)
		return 0;

	char* buf = KBEngine::strutil::wchar2char(s.GetBuffer(0));
	int32 uid = atoi(buf);
	free(buf);

	return uid;
}

Network::Address CguiconsoleDlg::getTreeItemAddr(HTREEITEM hItem)
{
	if(hItem == NULL)
		return Network::Address::NONE;

	CString s = m_tree.GetItemText(hItem);
	int fi_cellappmgr = s.Find(L"cellappmgr", 0);
	int fi_baseappmgr = s.Find(L"baseappmgr", 0);
	int fi_cellapp = s.Find(L"cellapp", 0);
	int fi_baseapp = s.Find(L"baseapp", 0);
	if(fi_cellappmgr >= 0)
		fi_cellapp = -1;
	if(fi_baseappmgr >= 0)
		fi_baseapp = -1;
	int fi_loginapp = s.Find(L"loginapp", 0);
	int fi_dbmgr = s.Find(L"dbmgr", 0);
	int fi_logger = s.Find(L"logger", 0);
	int fi_bots = s.Find(L"bots", 0);

	if(fi_cellapp  < 0 &&
		fi_baseapp < 0 &&
		fi_cellappmgr < 0 &&
		fi_baseappmgr < 0 &&
		fi_loginapp < 0 &&
		fi_logger < 0 &&
		fi_dbmgr < 0 &&
		fi_bots < 0)
	{
		return Network::Address::NONE;
	}

	char* buf = KBEngine::strutil::wchar2char(s.GetBuffer(0));
	std::string sbuf = buf;
	free(buf);

	std::string::size_type i = sbuf.find("[");
	std::string::size_type j = sbuf.find("]");
	sbuf = sbuf.substr(i + 1, j - 1);
	std::string::size_type k = sbuf.find(":");
	std::string sip, sport;
	sip = sbuf.substr(0, k);
	sport = sbuf.substr(k + 1, sbuf.find("]"));
	strutil::kbe_replace(sport, "]", "");

	std::map<CString, CString>::iterator mapiter = m_ipMappings.find(CString(sip.c_str()));
	if (mapiter != m_ipMappings.end())
	{
		buf = KBEngine::strutil::wchar2char(mapiter->second.GetBuffer(0));
		sip = buf;
		free(buf);
	}

	Network::EndPoint endpoint;
	u_int32_t address;
	Network::Address::string2ip(sip.c_str(), address);
	KBEngine::Network::Address addr(address, htons(atoi(sport.c_str())));
	return addr;
}

bool CguiconsoleDlg::connectTo()
{
	// TODO: Add your command handler code here
	HTREEITEM hItem = m_tree.GetSelectedItem(); 
	KBEngine::Network::Address addr = getTreeItemAddr(hItem);
	if(addr.ip == 0)
	{
		::AfxMessageBox(L"no select!");
		return false;
	}
	
	Network::EndPoint* endpoint = Network::EndPoint::createPoolObject(OBJECTPOOL_POINT);
	endpoint->socket(SOCK_STREAM);
	if (!endpoint->good())
	{
		AfxMessageBox(L"couldn't create a socket\n");
		return false;
	}

	endpoint->addr(addr);

	if(endpoint->connect(addr.port, addr.ip) == -1)
	{
		CString err;
		err.Format(L"connect server error! %d", ::WSAGetLastError());
		AfxMessageBox(err);
		return false;
	}

	endpoint->setnonblocking(true);
	Network::Channel* pChannel = _networkInterface.findChannel(endpoint->addr());
	if(pChannel)
	{
		_networkInterface.deregisterChannel(pChannel);
		pChannel->destroy();
		Network::Channel::reclaimPoolObject(pChannel);
	}

	pChannel = Network::Channel::createPoolObject(OBJECTPOOL_POINT);
	bool ret = pChannel->initialize(_networkInterface, endpoint, Network::Channel::INTERNAL);
	if(!ret)
	{
		ERROR_MSG(fmt::format("CguiconsoleDlg::connectTo: initialize({}) is failed!\n",
			pChannel->c_str()));

		pChannel->destroy();
		Network::Channel::reclaimPoolObject(pChannel);
		return 0;
	}

	pChannel->proxyID(getTreeItemComponent(m_tree.GetSelectedItem()));
	if(!_networkInterface.registerChannel(pChannel))
	{
		pChannel->destroy();
		Network::Channel::reclaimPoolObject(pChannel);

		CString err;
		err.Format(L"CguiconsoleDlg::connectTo: registerChannel(%s) is failed!\n",
			pChannel->c_str());
		AfxMessageBox(err);
		return false;
	}

	return true;
}

void CguiconsoleDlg::closeCurrTreeSelChannel()
{
	HTREEITEM hItem = m_tree.GetSelectedItem(); 
	KBEngine::Network::Address addr = getTreeItemAddr(hItem);
	if(addr.ip == 0)
	{
		::AfxMessageBox(L"no select!");
		return;
	}

	Network::Channel* pChannel = _networkInterface.findChannel(addr);
	if(pChannel)
	{
		_networkInterface.deregisterChannel(pChannel);
		pChannel->destroy();
		Network::Channel::reclaimPoolObject(pChannel);
	}
}

void CguiconsoleDlg::OnMenu_Update()
{
	_networkInterface.deregisterAllChannels();
	Components::getSingleton().clear();
	Components::getSingleton().delComponent(Components::ANY_UID, LOGGER_TYPE, 0, true, false);
	Components::getSingleton().delComponent(Components::ANY_UID, BOTS_TYPE, 0, true, false);
	::SetTimer(m_hWnd, 2, 100, NULL);
}

void CguiconsoleDlg::autoShowWindow()
{
	switch(m_tab.GetCurSel())
    {
    case 0:
		m_statusWnd.ShowWindow(SW_SHOW);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		m_graphsWindow.ShowWindow(SW_HIDE);
		break;
    case 1:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_SHOW);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		m_graphsWindow.ShowWindow(SW_HIDE);
		break;
    case 2:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_SHOW);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		m_graphsWindow.ShowWindow(SW_HIDE);
		break;
    case 3:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_SHOW);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		m_graphsWindow.ShowWindow(SW_HIDE);
		break;
    case 4:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_SHOW);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		m_graphsWindow.ShowWindow(SW_HIDE);
		break;
    case 5:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_SHOW);
		m_graphsWindow.ShowWindow(SW_HIDE);
		break;
    case 6:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		m_graphsWindow.ShowWindow(SW_SHOW);
		break;
    };
}

void CguiconsoleDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	autoShowWindow();
}

void CguiconsoleDlg::OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	TVHITTESTINFO hittestInfo;

	GetCursorPos(&hittestInfo.pt);     
	m_tree.ScreenToClient(&hittestInfo.pt);

	HTREEITEM hItem = m_tree.HitTest(&hittestInfo);   
	
	if(hItem == NULL || TVHT_NOWHERE & hittestInfo.flags)
		return;

	bool changeToChecked = false;

	// 复选框被选中就连接否则断开连接
	if(TVHT_ONITEMSTATEICON & hittestInfo.flags)
	{
		m_tree.SelectItem(hItem);
		BOOL checked = !m_tree.GetCheck(hItem);

		if(checked)
		{
			if(!connectTo())
				return;

			changeToChecked = true;
		}
		else
		{
			closeCurrTreeSelChannel();
		}
	}

	COMPONENT_TYPE debugComponentType = getTreeItemComponent(hItem);
	BOOL isread_only = debugComponentType != CELLAPP_TYPE && debugComponentType != BASEAPP_TYPE && debugComponentType != BOTS_TYPE;
	m_debugWnd.sendbufferWnd()->SetReadOnly(isread_only || !changeToChecked && !m_tree.GetCheck(hItem));

	if(!isread_only && changeToChecked)
	{
		CString s;
		m_debugWnd.displaybufferWnd()->GetWindowTextW(s);
		
		if(s.GetLength() <= 0)
			s += L">>>请在下面的窗口写python代码来调试服务端。\r\n>>>ctrl+enter 发送\r\n>>>↑↓使用历史命令\r\n\r\n";
		else
			s += L">>>";

		m_debugWnd.displaybufferWnd()->SetWindowTextW(s);
	}

	Network::Address currAddr = this->getTreeItemAddr(hItem);
	if(currAddr.ip == 0)
		return;

	CString title;
	wchar_t* tbuf = strutil::char2wchar(currAddr.c_str());
	title.Format(L"guiconsole : selected[%s]", tbuf);
	free(tbuf);
	this->SetWindowTextW(title.GetBuffer(0));

	m_watcherWnd.clearAllData();

	if(debugComponentType == LOGGER_TYPE && changeToChecked)
	{
		HTREEITEM hItem = m_tree.GetSelectedItem(); 
		KBEngine::Network::Address addr = getTreeItemAddr(hItem);
		m_logWnd.onConnectionState(changeToChecked, addr);
	}
}

void CguiconsoleDlg::OnConnectRemoteMachine()
{
	// TODO: Add your command handler code here
	CConnectRemoteMachineWindow dlg;
	dlg.DoModal();
}

void CguiconsoleDlg::OnHelpAbout()
{
	// TODO: Add your command handler code here
	::CAboutDlg dlg;
	dlg.DoModal();
}

void CguiconsoleDlg::OnToolBar_Find()
{
	OnConnectRemoteMachine();
}


void CguiconsoleDlg::OnToolBar_StartServer()
{
	/*
	COMPONENT_TYPE startComponentTypes[] = {BASEAPP_TYPE, CELLAPP_TYPE, BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, LOGINAPP_TYPE, DBMGR_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};
	
	int i = 0;

	while(1)
	{
		srand(KBEngine::getSystemTime());
		uint16 nport = KBE_PORT_START + (rand() % 1000);
		Network::BundleBroadcast bhandler(_networkInterface, nport);

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

		COMPONENT_TYPE componentType = startComponentTypes[i++];
		if(componentType == UNKNOWN_COMPONENT_TYPE)
			break;

		bhandler.newMessage(MachineInterface::startserver);
		bhandler << KBEngine::getUserUID();
		bhandler << componentType;

		uint32 ip = _networkInterface.intaddr().ip;
		uint16 port = bhandler.epListen().addr().port;
		bhandler << ip << port;

		if(!bhandler.broadcast())
		{
			ERROR_MSG("CguiconsoleDlg::OnToolBar_StartServer: broadcast error!\n");
			//::AfxMessageBox(L"不能发送服务器启动包。");
			break;
		}

		if(!bhandler.receive(NULL, 0, 1000000))
		{
			ERROR_MSG("CguiconsoleDlg::OnToolBar_StartServer: recv error!\n");
			//::AfxMessageBox(L"接收服务器启动包错误。");
			break;
		}
		
		bool success;
		bhandler >> success;
	}

	_networkInterface.deregisterAllChannels();
	Components::getSingleton().clear();
	Components::getSingleton().delComponent(Components::ANY_UID, LOGGER_TYPE, 0, true, false);
	Components::getSingleton().delComponent(Components::ANY_UID, BOTS_TYPE, 0, true, false);
	::SetTimer(m_hWnd, 2, 1000, NULL);
	*/


	CStartServerWindow dlg;
	dlg.DoModal();
}

void CguiconsoleDlg::OnToolBar_StopServer()
{
	COMPONENT_TYPE startComponentTypes[] = {BASEAPP_TYPE, CELLAPP_TYPE, BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, LOGINAPP_TYPE, DBMGR_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};
	
	int i = 0;

	while(1)
	{
		srand(KBEngine::getSystemTime());
		uint16 nport = KBE_PORT_START + (rand() % 1000);
		Network::BundleBroadcast bhandler(_networkInterface, nport);

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

		COMPONENT_TYPE componentType = startComponentTypes[i++];
		if(componentType == UNKNOWN_COMPONENT_TYPE)
			break;

		bhandler.newMessage(MachineInterface::stopserver);
		bhandler << KBEngine::getUserUID();
		bhandler << componentType;
		KBEngine::COMPONENT_ID cid = 0;
		bhandler << cid;
		
		uint32 ip = _networkInterface.intTcpAddr().ip;
		uint16 port = bhandler.epListen().addr().port;
		bhandler << ip << port;

		if(!bhandler.broadcast())
		{
			ERROR_MSG("CguiconsoleDlg::OnToolBar_StartServer: broadcast error!\n");
			//::AfxMessageBox(L"不能发送服务器启动包。");
			break;
		}

		if(!bhandler.receive(NULL, 0, 3000000))
		{
			ERROR_MSG("CguiconsoleDlg::OnToolBar_StartServer: recv error!\n");
			//::AfxMessageBox(L"接收服务器启动包错误。");
			break;
		}
		
		bool success;
		bhandler >> success;
	}
}

void CguiconsoleDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	g_isDestroyed = true;
	//threadPool_.finalise();
	CDialog::OnClose();
}

void CguiconsoleDlg::onReceiveWatcherData(MemoryStream& s)
{
	m_watcherWnd.onReceiveWatcherData(s);
}

bool CguiconsoleDlg::startProfile(std::string name, int8 type, uint32 timinglen)
{
	if(type == 0)
	{
		if(getTreeItemComponent(m_tree.GetSelectedItem()) != BASEAPP_TYPE && getTreeItemComponent(m_tree.GetSelectedItem()) != CELLAPP_TYPE
			&& getTreeItemComponent(m_tree.GetSelectedItem()) != BOTS_TYPE)
		{
			::AfxMessageBox(L"not support!");
			return false;
		}
	}

	Network::Channel* pChannel = _networkInterface.findChannel(this->getTreeItemAddr(m_tree.GetSelectedItem()));
	if(pChannel)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		if(getTreeItemComponent(m_tree.GetSelectedItem()) == BASEAPP_TYPE)
			(*pBundle).newMessage(BaseappInterface::startProfile);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == BASEAPPMGR_TYPE)
			(*pBundle).newMessage(BaseappmgrInterface::startProfile);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == CELLAPP_TYPE)
			(*pBundle).newMessage(CellappInterface::startProfile);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == CELLAPPMGR_TYPE)
			(*pBundle).newMessage(CellappmgrInterface::startProfile);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == DBMGR_TYPE)
			(*pBundle).newMessage(DbmgrInterface::startProfile);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == LOGINAPP_TYPE)
			(*pBundle).newMessage(LoginappInterface::startProfile);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == LOGGER_TYPE)
			(*pBundle).newMessage(LoggerInterface::startProfile);
		else if(getTreeItemComponent(m_tree.GetSelectedItem()) == BOTS_TYPE)
			(*pBundle).newMessage(BotsInterface::startProfile);
		else
		{
			::AfxMessageBox(L"not support!");
			Network::Bundle::reclaimPoolObject(pBundle);
			return false;
		}

		(*pBundle) << name;
		(*pBundle) << type;
		(*pBundle) << timinglen;
		pChannel->send(pBundle);
		return true;
	}

	return false;
}


void CguiconsoleDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	threadPool_.finalise();
}
