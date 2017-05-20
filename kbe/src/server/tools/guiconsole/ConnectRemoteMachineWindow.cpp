// ConnectRemoteMachineWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "ConnectRemoteMachineWindow.h"
#include "machine/machine_interface.h"
#include "server/components.h"
#include "helper/console_helper.h"

// CConnectRemoteMachineWindow dialog

IMPLEMENT_DYNAMIC(CConnectRemoteMachineWindow, CDialog)

CConnectRemoteMachineWindow::CConnectRemoteMachineWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectRemoteMachineWindow::IDD, pParent)
{

}

CConnectRemoteMachineWindow::~CConnectRemoteMachineWindow()
{
}

void CConnectRemoteMachineWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ip);
	DDX_Control(pDX, IDC_EDIT2, m_port);
	DDX_Control(pDX, IDC_LIST1, m_log);
	DDX_Control(pDX, IDC_IPADDRESS2, m_lan_ip);
	DDX_Control(pDX, IDC_IPADDRESS3, m_internet_ip);
	DDX_Control(pDX, IDC_LIST2, m_mappinglog);
}


BEGIN_MESSAGE_MAP(CConnectRemoteMachineWindow, CDialog)
	ON_BN_CLICKED(IDOK, &CConnectRemoteMachineWindow::OnBnClickedOk)
	ON_LBN_DBLCLK(IDC_LIST1, &CConnectRemoteMachineWindow::OnLbnDblclkList1)
	ON_BN_CLICKED(IDC_ADD_IPMAPPING, &CConnectRemoteMachineWindow::OnBnClickedAddIpmapping)
	ON_BN_CLICKED(IDC_DEL_IPMAPPING, &CConnectRemoteMachineWindow::OnBnClickedDelIpmapping)
	ON_LBN_SELCHANGE(IDC_LIST1, &CConnectRemoteMachineWindow::OnLbnSelchangeList1)
	ON_LBN_DBLCLK(IDC_LIST2, &CConnectRemoteMachineWindow::OnLbnDblclkList2)
END_MESSAGE_MAP()

BOOL CConnectRemoteMachineWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_port.SetWindowTextW(L"20099");

	loadHistory();

	int i = 0;
	std::deque<CString>::iterator iter = m_historyCommand.begin();
	for(; iter != m_historyCommand.end(); iter++)
	{
		if(i == 0)
		{
			CString output = L"";
			CString output1 = L"";
			AfxExtractSubString(output, (*iter), 0, _T(':'));
			AfxExtractSubString(output1, (*iter), 1, _T(':'));

			char* ip = KBEngine::strutil::wchar2char(output.GetBuffer(0));
			m_ip.SetAddress(ntohl(inet_addr(ip)));
			free(ip);

			m_port.SetWindowTextW(output1);
		}

		i++;
		m_log.AddString((*iter));
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}
// CConnectRemoteMachineWindow message handlers

void CConnectRemoteMachineWindow::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	byte ips[4];

	if (0 == m_ip.GetAddress(ips[0],ips[1],ips[2],ips[3]))
	{
		AfxMessageBox(L"address is error!");
		return;
	}
	
	char strip[256];
	sprintf_s(strip, 256, "%d.%d.%d.%d", ips[0],ips[1],ips[2],ips[3]);
	
	KBEngine::u_int16_t port = 0;
	CString sport;
	m_port.GetWindowTextW(sport);
	char* csport = KBEngine::strutil::wchar2char(sport.GetBuffer(0));
	port = atoi(csport);
	std::string command = strip;
	command += ":";
	command += csport;
	free(csport);

	KBEngine::Network::EndPoint* endpoint = KBEngine::Network::EndPoint::createPoolObject();

	KBEngine::u_int32_t address;
	Network::Address::string2ip(strip, address);
	KBEngine::Network::Address addr(address, htons(port));

	if(addr.ip == 0)
	{
		::AfxMessageBox(L"address is error!");
		KBEngine::Network::EndPoint::reclaimPoolObject(endpoint);
		return;
	}

	endpoint->socket(SOCK_STREAM);
	if (!endpoint->good())
	{
		AfxMessageBox(L"couldn't create a socket\n");
		KBEngine::Network::EndPoint::reclaimPoolObject(endpoint);
		return;
	}

	endpoint->addr(addr);
	if(endpoint->connect(addr.port, addr.ip) == -1)
	{
		CString err;
		err.Format(L"connect server is error! %d", ::WSAGetLastError());
		AfxMessageBox(err);
		KBEngine::Network::EndPoint::reclaimPoolObject(endpoint);
		return;
	}

	endpoint->setnonblocking(false);
	int8 findComponentTypes[] = {LOGGER_TYPE, BASEAPP_TYPE, CELLAPP_TYPE, BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, LOGINAPP_TYPE, DBMGR_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};
	int ifind = 0;

	while(true)
	{
		int8 findComponentType = findComponentTypes[ifind++];
		if(findComponentType == UNKNOWN_COMPONENT_TYPE)
		{
			//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
			//	COMPONENT_NAME_EX(findComponentType));
			break;
		}

		KBEngine::Network::Bundle bhandler;
		bhandler.newMessage(KBEngine::MachineInterface::onFindInterfaceAddr);

		KBEngine::MachineInterface::onFindInterfaceAddrArgs7::staticAddToBundle(bhandler, KBEngine::getUserUID(), KBEngine::getUsername(), 
			CONSOLE_TYPE, g_componentID, (COMPONENT_TYPE)findComponentType, 0, 0);

		endpoint->send(&bhandler);

		KBEngine::Network::TCPPacket packet;
		packet.resize(65535);

		endpoint->setnonblocking(true);
		KBEngine::sleep(300);
		packet.wpos(endpoint->recv(packet.data(), 65535));

		while(packet.length() > 0)
		{
			MachineInterface::onBroadcastInterfaceArgs25 args;
			
			try
			{
				args.createFromStream(packet);
			}catch(MemoryStreamException &)
			{
				goto END;
			}

			INFO_MSG(fmt::format("CConnectRemoteMachineWindow::OnBnClickedOk: found {}, addr:{}:{}\n",
				COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType), inet_ntoa((struct in_addr&)args.intaddr), ntohs(args.intport)));

			Components::getSingleton().addComponent(args.uid, args.username.c_str(), 
				(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.globalorderid, args.grouporderid, args.gus,
				args.intaddr, args.intport, args.extaddr, args.extport, args.extaddrEx, args.pid, args.cpu, args.mem, args.usedmem, 
				args.extradata, args.extradata1, args.extradata2, args.extradata3);

		}
	}
END:
	dlg->updateTree();

	KBEngine::Network::EndPoint::reclaimPoolObject(endpoint);
	wchar_t* wcommand = KBEngine::strutil::char2wchar(command.c_str());
	bool found = false;
	std::deque<CString>::iterator iter = m_historyCommand.begin();
	for(; iter != m_historyCommand.end(); iter++)
	{
		if((*iter) == wcommand)
		{
			found = true;
			break;
		}
	}
	
	if(!found)
	{
		m_historyCommand.push_front(wcommand);
		if(m_historyCommand.size() > 10)
			m_historyCommand.pop_back();

		saveHistory();
	}

	free(wcommand);

	OnOK(); 
}

void CConnectRemoteMachineWindow::saveHistory()
{
    //����һ��XML���ĵ�����
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
    CString fullPath = appPath + L"\\histroycommands1.xml";

	char fname[4096] = {0};

	int len = WideCharToMultiByte(CP_ACP, 0, fullPath, fullPath.GetLength(), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP,0, fullPath, fullPath.GetLength(), fname, len, NULL, NULL);
	fname[len + 1] = '\0';

	pDocument->SaveFile(fname);

	m_log.ResetContent();
	std::deque<CString>::iterator iter1 = m_historyCommand.begin();
	for(; iter1 != m_historyCommand.end(); iter1++)
	{
		m_log.AddString((*iter1));
	}
}

void CConnectRemoteMachineWindow::loadHistory()
{
    CString appPath = GetAppPath();
    CString fullPath = appPath + L"\\histroycommands1.xml";

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
			wchar_t* strCommand = strutil::char2wchar(c.c_str());
			m_historyCommand.push_back(strCommand);
			free(strCommand);
		}while((node = node->NextSibling()));												
	}

	pDocument->Clear();
	delete pDocument;
}	

void CConnectRemoteMachineWindow::OnLbnDblclkList1()
{
	// TODO: Add your control notification handler code here
	CString str;
	m_log.GetText(m_log.GetCurSel(), str);
	
	CString output = L"";
	CString output1 = L"";
	AfxExtractSubString(output, str, 0, _T(':'));
	AfxExtractSubString(output1, str, 1, _T(':'));

	char* ip = KBEngine::strutil::wchar2char(output.GetBuffer(0));
	m_ip.SetAddress(ntohl(inet_addr(ip)));
	free(ip);

	m_port.SetWindowTextW(output1);
}


void CConnectRemoteMachineWindow::OnBnClickedAddIpmapping()
{
	// TODO: Add your control notification handler code here
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	byte lan_ips[4];

	if (0 == m_lan_ip.GetAddress(lan_ips[0], lan_ips[1], lan_ips[2], lan_ips[3]))
	{
		AfxMessageBox(L"LAN-address error!");
		return;
	}

	byte internet_ips[4];

	if (0 == m_internet_ip.GetAddress(internet_ips[0], internet_ips[1], internet_ips[2], internet_ips[3]))
	{
		AfxMessageBox(L"Internet-address error!");
		return;
	}

	char str_lanip[256];
	sprintf_s(str_lanip, 256, "%d.%d.%d.%d", lan_ips[0], lan_ips[1], lan_ips[2], lan_ips[3]);

	char str_internetip[256];
	sprintf_s(str_internetip, 256, "%d.%d.%d.%d", internet_ips[0], internet_ips[1], internet_ips[2], internet_ips[3]);

	dlg->m_ipMappings[CString(str_lanip)] = CString(str_internetip);
	updateMappingListCtrl();
}


void CConnectRemoteMachineWindow::updateMappingListCtrl()
{
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);

	m_mappinglog.ResetContent();

	std::map<CString, CString>::iterator iter = dlg->m_ipMappings.begin();
	for (; iter != dlg->m_ipMappings.end(); ++iter)
	{
		m_mappinglog.AddString(iter->first + L">" + iter->second);
	}
}

void CConnectRemoteMachineWindow::OnBnClickedDelIpmapping()
{
	// TODO: Add your control notification handler code here
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);

	CString str;
	m_mappinglog.GetText(m_mappinglog.GetCurSel(), str);

	CString output = L"";
	CString output1 = L"";
	AfxExtractSubString(output, str, 0, _T('>'));
	AfxExtractSubString(output1, str, 1, _T('>'));
	
	dlg->m_ipMappings.erase(output);
	updateMappingListCtrl();
}


void CConnectRemoteMachineWindow::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
}


void CConnectRemoteMachineWindow::OnLbnDblclkList2()
{
	// TODO: Add your control notification handler code here
}
