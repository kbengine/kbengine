
// guiconsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "network/bundle_broadcast.hpp"
#include "network/message_handler.hpp"
#include "server/components.hpp"
#include "helper/console_helper.hpp"
#include "xmlplus/xmlplus.hpp"

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

#undef DEFINE_IN_INTERFACE
#include "resourcemgr/resourcemgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "resourcemgr/resourcemgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/message_log/messagelog_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/message_log/messagelog_interface.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace KBEngine{
namespace ConsoleInterface{
	Mercury::MessageHandlers messageHandlers;
}
}

BOOL g_sendData = FALSE;

class ConsoleExecCommandCBMessageHandlerEx : public KBEngine::ConsoleInterface::ConsoleExecCommandCBMessageHandler
{
public:
	virtual void handle(Mercury::Channel* pChannel, MemoryStream& s)
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		std::string strarg;	
		s >> strarg;
		dlg->onExecScriptCommandCB(pChannel, strarg);
	};
};

class ConsoleLogMessageHandlerEx : public KBEngine::ConsoleInterface::ConsoleLogMessageHandler
{
public:
	virtual void handle(Mercury::Channel* pChannel, MemoryStream& s)
	{
		CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
		std::string str;
		s >> str;
		dlg->onReceiveRemoteLog(str);
	};
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
	m_isInit(false),
	m_historyCommand(),
	m_historyCommandIndex(0),
	m_isUsingHistroy(false)
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
	_dispatcher.breakProcessing(false);

	::SetTimer(m_hWnd, 1, 100, NULL);
	::SetTimer(m_hWnd, 2, 100, NULL);
	::SetTimer(m_hWnd, 3, 1000 * 20, NULL);

	m_isInit = true;
	
	m_tab.InsertItem(0, _T("STATUS"), 0); 
	m_statusWnd.Create(IDD_STATUS, GetDlgItem(IDC_TAB1));
	
	m_tab.InsertItem(1, _T("DEBUG"), 0); 
	m_debugWnd.Create(IDD_DEBUG, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(2, _T("LOG"), 0); 
	m_logWnd.Create(IDD_LOG, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(3, _T("profile"), 0); 
	m_profileWnd.Create(IDD_PROFILE, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(4, _T("watcher"), 0); 
	m_watcherWnd.Create(IDD_WATCHER, GetDlgItem(IDC_TAB1));

	m_tab.InsertItem(5, _T("spaceview"), 0); 
	m_spaceViewWnd.Create(IDD_SPACEVIEW, GetDlgItem(IDC_TAB1));

	DWORD styles = ::GetWindowLong(m_tree.m_hWnd, GWL_STYLE);
	styles |= TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
	::SetWindowLong(m_tree.m_hWnd, GWL_STYLE, styles);

	loadHistory();
	autoWndSize();
	updateTree();

	KBEngine::ConsoleInterface::messageHandlers.add("Console::onExecScriptCommandCB", new KBEngine::ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1, MERCURY_VARIABLE_MESSAGE, 
		new ConsoleExecCommandCBMessageHandlerEx);

	KBEngine::ConsoleInterface::messageHandlers.add("Console::onReceiveRemoteLog", new KBEngine::ConsoleInterface::ConsoleLogMessageHandlerArgsStream, MERCURY_VARIABLE_MESSAGE, 
		new ConsoleLogMessageHandlerEx);
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
				else
					return NULL;
			}

			childItem = m_tree.GetNextItem(childItem, TVGN_NEXT);
		}
	}while((rootitem = m_tree.GetNextItem(rootitem, TVGN_NEXT)) != NULL);

	return NULL;
}

void CguiconsoleDlg::commitPythonCommand(CString strCommand)
{
	if(getTreeItemComponent(m_tree.GetSelectedItem()) != CELLAPP_TYPE 
		&& getTreeItemComponent(m_tree.GetSelectedItem()) != BASEAPP_TYPE)
	{
		::AfxMessageBox(L"the component can not debug!");
		return;
	}
	
	m_isUsingHistroy = false;
	char buffer[4096] = {0};

	int len = WideCharToMultiByte(CP_ACP, 0, strCommand, strCommand.GetLength(), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP,0, strCommand, strCommand.GetLength(), buffer, len, NULL, NULL);
	buffer[len + 1] = '\0';

	int charLen = strlen(buffer);
	std::string s = buffer;

	m_historyCommand.push_back(strCommand);
	historyCommandCheck();
	m_historyCommandIndex = m_historyCommand.size() - 1;

	int i = 0;
	while((i = s.rfind("\r")) != -1)
		s.erase(i, 1);

	// 对普通的输入加入print 让服务器回显信息
    if((s.find("=")) == -1 &&
		(s.find("print")) == -1 &&
		(s.find("import")) == -1 &&
		(s.find("class")) == -1 &&
		(s.find("def")) == -1 &&
		(s.find("del")) == -1 &&
		(s.find("if")) == -1 &&
		(s.find("for")) == -1 &&
		(s.find("while")) == -1
		)
        s = "print(" + s + ")";

	Mercury::Channel* pChannel = _networkInterface.findChannel(this->getTreeItemAddr(m_tree.GetSelectedItem()));
	if(pChannel)
	{
		Mercury::Bundle bundle;
		if(getTreeItemComponent(m_tree.GetSelectedItem()) == BASEAPP_TYPE)
			bundle.newMessage(BaseappInterface::onExecScriptCommand);
		else
			bundle.newMessage(CellappInterface::onExecScriptCommand);

		bundle << s;
		bundle.send(this->getNetworkInterface(), pChannel);

		int len = MultiByteToWideChar(CP_ACP,0, buffer, charLen, NULL,0);
		wchar_t *buf = new wchar_t[len + 1];
		MultiByteToWideChar(CP_ACP, 0, buffer, charLen, buf, len);
		buf[len] = L'\0';

		CString str1, str2;
		m_debugWnd.displaybufferWnd()->GetWindowText(str2);
		wchar_t sign[10] = {L">>>"};
		str1.Append(sign);
		str1.Append(buf);
		delete[] buf;
		g_sendData = TRUE;
		str2 += str1;
		m_debugWnd.displaybufferWnd()->SetWindowTextW(str2);

		saveHistory();
	}
}

CString GetAppPath()
{
	::TCHAR modulePath[MAX_PATH];
    GetModuleFileName(NULL, modulePath, MAX_PATH);
    CString strModulePath(modulePath);
    strModulePath = strModulePath.Left(strModulePath.ReverseFind(_T('\\')));
    return strModulePath;
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

		char buffer[4096] = {0};
		CString strCommand = (*iter);

		int len = WideCharToMultiByte(CP_ACP, 0, strCommand, strCommand.GetLength(), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP,0, strCommand, strCommand.GetLength(), buffer, len, NULL, NULL);
		buffer[len + 1] = '\0';


		TiXmlText *content = new TiXmlText(buffer);
		rootElementChild->LinkEndChild(content);
	}

    CString appPath = GetAppPath();
    CString fullPath = appPath + L"\\histroycommands.xml";

	char fname[4096] = {0};

	int len = WideCharToMultiByte(CP_ACP, 0, fullPath, fullPath.GetLength(), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP,0, fullPath, fullPath.GetLength(), fname, len, NULL, NULL);
	fname[len + 1] = '\0';

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
			std::string c = node->FirstChild()->Value();
			wchar_t* strCommand = char2wchar(c.c_str());
			m_historyCommand.push_back(strCommand);
			free(strCommand);
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
		_dispatcher.processOnce(false);
		_networkInterface.handleChannels(&KBEngine::ConsoleInterface::messageHandlers);
		break;
	case 2:
		{
			int8 findComponentTypes[] = {MESSAGELOG_TYPE, RESOURCEMGR_TYPE, BASEAPP_TYPE, CELLAPP_TYPE, BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, LOGINAPP_TYPE, DBMGR_TYPE, UNKNOWN_COMPONENT_TYPE};
			int ifind = 0;

			while(true)
			{
				int8 findComponentType = findComponentTypes[ifind];
				if(findComponentType == UNKNOWN_COMPONENT_TYPE)
				{
					//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
					//	COMPONENT_NAME_EX(findComponentType));
					
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
						//	COMPONENT_NAME_EX(findComponentType));
						
						::KillTimer(m_hWnd, nIDEvent);
						return;
					}

					INFO_MSG("CguiconsoleDlg::OnTimer: found %s, addr:%s:%u\n",
						COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType), inet_ntoa((struct in_addr&)args.intaddr), ntohs(args.intaddr));

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
						ERROR_MSG("CguiconsoleDlg::OnTimer: %s not found. receive data is error!\n", COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType));
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
	case 3:
		{
			const Mercury::NetworkInterface::ChannelMap& channels = _networkInterface.channels();
			Mercury::NetworkInterface::ChannelMap::const_iterator iter = channels.begin();
			for(; iter != channels.end(); iter++)
			{
				Mercury::Channel* pChannel = const_cast<KBEngine::Mercury::Channel*>(iter->second);
				Mercury::Bundle bundle;
				COMMON_MERCURY_MESSAGE(pChannel->proxyID(), bundle, onAppActiveTick);
				
				bundle << _componentType;
				bundle << _componentID;
				bundle.send(getNetworkInterface(), pChannel);
				pChannel->updateLastReceivedTime();
			}
		}
		break;
	default:
		break;
	};	
}

void CguiconsoleDlg::onExecScriptCommandCB(Mercury::Channel* pChannel, std::string& command)
{
	DEBUG_MSG("CguiconsoleDlg::onExecScriptCommandCB: %s\n", command.c_str());
	CString str;
	CEdit* lpEdit = (CEdit*)m_debugWnd.displaybufferWnd();
	lpEdit->GetWindowText(str);

	if(str.GetLength() > 0)
		str += "\r\n";
	else
		str += "";

	for(unsigned int i=0; i<command.size(); i++)
	{
		char c = command.c_str()[i];
		switch(c)
		{
		case 10:
			str += "\r\n";
			break;
		case 32:
			str += c;
			break;
		case 34:
			str += "\t";
			break;
		default:
			str += c;
		}
	}

	str += "\r\n";
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
	Components::COMPONENTS cts6 = Components::getSingleton().getComponents(MESSAGELOG_TYPE);
	Components::COMPONENTS cts7 = Components::getSingleton().getComponents(RESOURCEMGR_TYPE);
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
			kbe_snprintf(sbuf, 1024, "%s[%s]", COMPONENT_NAME_EX(cinfos.componentType), cinfos.pIntAddr->c_str());
			wchar_t* wbuf = KBEngine::char2wchar(sbuf);
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
	int fi_messagelog = s.Find(L"messagelog", 0);
	int fi_resourcemgr = s.Find(L"resourcemgr", 0);

	if(fi_cellapp  < 0 &&
		fi_baseapp < 0 &&
		fi_cellappmgr < 0 &&
		fi_baseappmgr < 0 &&
		fi_loginapp < 0 &&
		fi_messagelog < 0 &&
		fi_resourcemgr < 0 &&
		fi_dbmgr < 0)
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
	else if(fi_messagelog >= 0)
	{
		return MESSAGELOG_TYPE;
	}
	else if(fi_resourcemgr >= 0)
	{
		return RESOURCEMGR_TYPE;
	}

	return UNKNOWN_COMPONENT_TYPE;
}

Mercury::Address CguiconsoleDlg::getTreeItemAddr(HTREEITEM hItem)
{
	if(hItem == NULL)
		return Mercury::Address::NONE;

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
	int fi_messagelog = s.Find(L"messagelog", 0);
	int fi_resourcemgr = s.Find(L"resourcemgr", 0);

	if(fi_cellapp  < 0 &&
		fi_baseapp < 0 &&
		fi_cellappmgr < 0 &&
		fi_baseappmgr < 0 &&
		fi_loginapp < 0 &&
		fi_messagelog < 0 &&
		fi_resourcemgr < 0 &&
		fi_dbmgr < 0)
	{
		return Mercury::Address::NONE;
	}

	char* buf = KBEngine::wchar2char(s.GetBuffer(0));
	std::string sbuf = buf;

	std::string::size_type i = sbuf.find("[");
	std::string::size_type j = sbuf.find("]");
	sbuf = sbuf.substr(i + 1, j - 1);
	std::string::size_type k = sbuf.find(":");
	std::string sip, sport;
	sip = sbuf.substr(0, k);
	sport = sbuf.substr(k + 1, sbuf.find("]"));
	sport = kbe_replace(sport, "]", "");

	Mercury::EndPoint endpoint;
	u_int32_t address;
	endpoint.convertAddress(sip.c_str(), address);
	KBEngine::Mercury::Address addr(address, htons(atoi(sport.c_str())));
	return addr;
}

void CguiconsoleDlg::connectTo()
{
	// TODO: Add your command handler code here
	HTREEITEM hItem = m_tree.GetSelectedItem(); 
	KBEngine::Mercury::Address addr = getTreeItemAddr(hItem);
	if(addr.ip == 0)
	{
		::AfxMessageBox(L"no select!");
		return;
	}
	
	Mercury::EndPoint* endpoint = new Mercury::EndPoint();
	endpoint->socket(SOCK_STREAM);
	if (!endpoint->good())
	{
		AfxMessageBox(L"couldn't create a socket\n");
		return;
	}

	endpoint->addr(addr);

	if(endpoint->connect(addr.port, addr.ip) == -1)
	{
		CString err;
		err.Format(L"connect server is error! %d", ::WSAGetLastError());
		AfxMessageBox(err);
		return;
	}

	endpoint->setnonblocking(true);
	Mercury::Channel* pChannel = _networkInterface.findChannel(endpoint->addr());
	if(pChannel)
	{
		_networkInterface.deregisterChannel(pChannel);
		pChannel->destroy();
	}

	pChannel = new Mercury::Channel(_networkInterface, endpoint, Mercury::Channel::INTERNAL);
	pChannel->proxyID(getTreeItemComponent(m_tree.GetSelectedItem()));
	if(!_networkInterface.registerChannel(pChannel))
	{
		CString err;
		err.Format(L"ListenerReceiver::handleInputNotification:registerChannel(%s) is failed!\n",
			pChannel->c_str());
		AfxMessageBox(err);
		return;
	}
}

void CguiconsoleDlg::closeCurrTreeSelChannel()
{
	HTREEITEM hItem = m_tree.GetSelectedItem(); 
	KBEngine::Mercury::Address addr = getTreeItemAddr(hItem);
	if(addr.ip == 0)
	{
		::AfxMessageBox(L"no select!");
		return;
	}

	Mercury::Channel* pChannel = _networkInterface.findChannel(addr);
	if(pChannel)
	{
		_networkInterface.deregisterChannel(pChannel);
		pChannel->destroy();
	}
}

void CguiconsoleDlg::OnMenu_Update()
{
	_networkInterface.deregisterAllChannels();
	Components::getSingleton().clear();

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
		break;
    case 1:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_SHOW);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		break;
    case 2:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_SHOW);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		break;
    case 3:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_SHOW);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		break;
    case 4:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_SHOW);
		m_spaceViewWnd.ShowWindow(SW_HIDE);
		break;
    case 5:
		m_statusWnd.ShowWindow(SW_HIDE);
		m_debugWnd.ShowWindow(SW_HIDE);
		m_logWnd.ShowWindow(SW_HIDE);
		m_profileWnd.ShowWindow(SW_HIDE);
		m_watcherWnd.ShowWindow(SW_HIDE);
		m_spaceViewWnd.ShowWindow(SW_SHOW);
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
			connectTo();
			changeToChecked = true;
		}
		else
		{
			closeCurrTreeSelChannel();
		}
	}

	COMPONENT_TYPE debugComponentType = getTreeItemComponent(hItem);
	BOOL isread_only = debugComponentType != CELLAPP_TYPE && debugComponentType != BASEAPP_TYPE;
	m_debugWnd.sendbufferWnd()->SetReadOnly(isread_only || !changeToChecked && !m_tree.GetCheck(hItem));

	if(!isread_only && changeToChecked)
	{
		CString s;
		m_debugWnd.displaybufferWnd()->GetWindowTextW(s);
		
		if(s.GetLength() <= 0)
			s += L">>>请在下面的窗口写python代码来调试服务器。\r\n>>>ctrl+enter 发送\r\n>>>↑↓使用历史命令\r\n\r\n";
		else
			s += L">>>";

		m_debugWnd.displaybufferWnd()->SetWindowTextW(s);
	}

	Mercury::Address currAddr = this->getTreeItemAddr(hItem);
	if(currAddr.ip == 0)
		return;

	CString title;
	wchar_t* tbuf = char2wchar(currAddr.c_str());
	title.Format(L"guiconsole : selected[%s]", tbuf);
	free(tbuf);
	this->SetWindowTextW(title.GetBuffer(0));

}

void CguiconsoleDlg::OnConnectRemoteMachine()
{
	// TODO: Add your command handler code here
}
