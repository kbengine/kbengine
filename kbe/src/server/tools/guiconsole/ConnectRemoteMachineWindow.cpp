// ConnectRemoteMachineWindow.cpp : implementation file
//

#include "stdafx.h"
#include "guiconsole.h"
#include "guiconsoleDlg.h"
#include "ConnectRemoteMachineWindow.h"
#include "machine/machine_interface.hpp"
#include "server/components.hpp"
#include "helper/console_helper.hpp"

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
}


BEGIN_MESSAGE_MAP(CConnectRemoteMachineWindow, CDialog)
	ON_BN_CLICKED(IDOK, &CConnectRemoteMachineWindow::OnBnClickedOk)
END_MESSAGE_MAP()


// CConnectRemoteMachineWindow message handlers

void CConnectRemoteMachineWindow::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CguiconsoleDlg* dlg = static_cast<CguiconsoleDlg*>(theApp.m_pMainWnd);
	byte ips[4];

	if (0 == m_ip.GetAddress(ips[0],ips[1],ips[2],ips[3]))
	{
		AfxMessageBox(L"address is error!");
	}
	
	char strip[256];
	sprintf_s(strip, 256, "%d.%d.%d.%d", ips[0],ips[1],ips[2],ips[3]);
	
	KBEngine::u_int16_t port = 0;
	CString sport;
	m_port.GetWindowTextW(sport);
	char* csport = KBEngine::wchar2char(sport.GetBuffer(0));
	port = atoi(csport);
	free(csport);

	KBEngine::Mercury::EndPoint* endpoint = new KBEngine::Mercury::EndPoint();

	KBEngine::u_int32_t address;
	endpoint->convertAddress(strip, address);
	KBEngine::Mercury::Address addr(address, htons(port));

	if(addr.ip == 0)
	{
		::AfxMessageBox(L"address is error!");
		delete endpoint;
		return;
	}

	endpoint->socket(SOCK_STREAM);
	if (!endpoint->good())
	{
		AfxMessageBox(L"couldn't create a socket\n");
		delete endpoint;
		return;
	}

	endpoint->addr(addr);
	if(endpoint->connect(addr.port, addr.ip) == -1)
	{
		CString err;
		err.Format(L"connect server is error! %d", ::WSAGetLastError());
		AfxMessageBox(err);
		delete endpoint;
		return;
	}

	endpoint->setnonblocking(false);
	int8 findComponentTypes[] = {MESSAGELOG_TYPE, RESOURCEMGR_TYPE, BASEAPP_TYPE, CELLAPP_TYPE, BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, LOGINAPP_TYPE, DBMGR_TYPE, UNKNOWN_COMPONENT_TYPE};
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

		KBEngine::Mercury::Bundle bhandler;
		bhandler.newMessage(KBEngine::MachineInterface::onFindInterfaceAddr);
		KBEngine::MachineInterface::onFindInterfaceAddrArgs6::staticAddToBundle(bhandler, KBEngine::getUserUID(), KBEngine::getUsername(), 
			CONSOLE_TYPE, findComponentType, 0, 0);

		bhandler.send(*endpoint);

		KBEngine::Mercury::TCPPacket packet;
		packet.resize(65535);
		packet.wpos(endpoint->recv(packet.data(), 65535));

		MachineInterface::onBroadcastInterfaceArgs8 args;
		args.createFromStream(packet);

		INFO_MSG("CConnectRemoteMachineWindow::OnBnClickedOk: found %s, addr:%s:%u\n",
			COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType), inet_ntoa((struct in_addr&)args.intaddr), ntohs(args.intaddr));

		Components::getSingleton().addComponent(args.uid, args.username.c_str(), 
			(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.intaddr, args.intport, args.extaddr, args.extport);

		dlg->updateTree();
	}

	delete endpoint;
	OnOK(); 
}
